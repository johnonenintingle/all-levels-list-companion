#include "ALLManager.hpp"

ALLManager& ALLManager::get() {
    static ALLManager instance;
    return instance;
}

void ALLManager::login(std::string_view username, std::string_view password, LoginCallback callback) {
    if (m_isLoggingIn || this->isLoggedIn()) {
        return;
    }

    auto body = matjson::Value{};

    body["username"] = username;
    body["password"] = password;

    m_isLoggingIn = true;

    m_loginCallbacks.push_back(std::move(callback));

    async::spawn(
        web::WebRequest()
            .bodyJSON(body)
            .post(fmt::format("{}/api/auth/login", getBaseURL())),
        [this](web::WebResponse resp) {
            auto code = resp.code();

            m_isLoggingIn = false;

            const auto onFailure = [this](std::string_view err) {
                for (int i = 0; i < m_loginCallbacks.size(); i++) {
                    m_loginCallbacks[i](Err(err));
                }
                
                m_loginCallbacks.clear();
            };

            auto res = resp.json();

            if (!res.isOk()) {
                onFailure(fmt::format("Failed to login, HTTP {}", code));
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                onFailure(json["error"].asString().unwrapOr(fmt::format("Failed to login, HTTP {}", code)));
                return;
            }

            auto token = json["token"].asString().unwrapOr("");

            if (token.empty() || !json.contains("user")) {
                onFailure("An unknown error has occurred");
                log::error("Failed to login, HTTP {}", code);
                return;
            }

            this->setToken(token);

            this->setUser(UserInfo{
                .id = static_cast<int>(json["user"]["id"].asInt().unwrapOr(0)),
                .username = json["user"]["username"].asString().unwrapOr(""),
                .avatarUrl = fmt::format("{}{}", getBaseURL(), json["user"]["avatarUrl"].asString().unwrapOr(""))
            });

            for (int i = 0; i < m_loginCallbacks.size(); i++) {
                m_loginCallbacks[i](Ok());
            }

            m_loginCallbacks.clear();

            this->linkGDAccount();
            this->syncAllCompletions();

            Notification::create("Succesfully logged in", NotificationIcon::Success)->show();
        }
    );
}

bool ALLManager::isLoggingIn() {
    return m_isLoggingIn;
}

void ALLManager::listenForLogin(LoginCallback callback) {
    if (m_isLoggingIn) {
        m_loginCallbacks.push_back(std::move(callback));
    }
}

bool ALLManager::isLoggedIn() {
    return !m_token.empty() && m_user.id > 0;
}

void ALLManager::setToken(std::string token) {
    m_token = std::move(token);
    Mod::get()->setSavedValue("account-token", m_token);
}

void ALLManager::setUser(UserInfo user) {
    m_user = std::move(user);

    Mod::get()->setSavedValue("account-id", m_user.id);
    Mod::get()->setSavedValue("account-username", m_user.username);
    Mod::get()->setSavedValue("account-avatar-url", m_user.avatarUrl);
}

UserInfo ALLManager::getUser() {
    return m_user;
}

bool ALLManager::isGDAccountLinked() {
    return m_linkedGDAccountID > 0 && m_linkedGDAccountID == GJAccountManager::get()->m_accountID;
}

void ALLManager::linkGDAccount() {
    if (this->isGDAccountLinked() || !this->isLoggedIn()) {
        return;
    }

    auto am = GJAccountManager::get();
    auto id = am->m_accountID;

    if (id <= 0) {
        return;
    }

    std::string username = GJAccountManager::get()->m_username;

    this->setLinkedGDAccount(id, username);

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(matjson::makeObject({
                { "accountId", id }
            }))
            .get(fmt::format("{}/api/users/gd-link", getBaseURL())),
        [this](web::WebResponse resp) {
            auto code = resp.code();

            if (code != 200) {
                if (code == 401) {
                    this->logout();
                    m_sessionExpired = true;
                }
                
                this->setLinkedGDAccount(0);
            }
        }
    );
}

void ALLManager::setLinkedGDAccount(int id, std::string username) {
    Mod::get()->setSavedValue("linked-gd-account-id", id);
    Mod::get()->setSavedValue("linked-gd-account-username", username);
    m_linkedGDAccountID = id;
    m_linkedGDAccountUsername = std::move(username);
}

void ALLManager::logout() {
    this->setToken("");
    m_user = UserInfo{};
    this->setLinkedGDAccount(0);
}

void ALLManager::updateUserInfo() {
    if (!this->isLoggedIn()) {
        return;
    }

    async::spawn(
        web::WebRequest()
            .get(fmt::format("{}/api/users/{}", getBaseURL(), this->getUser().id)),
        [this](web::WebResponse resp) {
            auto code = resp.code();

            if (code != 200) {
                log::error("User request failed, HTTP {}", code);

                return;
            }

            auto res = resp.json();

            if (!res.isOk()) {
                return;
            }

            auto json = res.unwrap();

            auto id = static_cast<int>(json["id"].asInt().unwrapOr(0));

            if (id <= 0) {
                return;
            }

            this->setUser(UserInfo{
                .id = id,
                .username = json["username"].asString().unwrapOr(""),
                .avatarUrl = fmt::format("{}{}", getBaseURL(), json["avatarUrl"].asString().unwrapOr(""))
            });
        }
    );
}

void ALLManager::syncPendingCompletions(bool silent) {
    if (this->isLoggedIn()) {
        auto vec = std::vector<int64_t>{};
        vec.reserve(m_pendingCompletions.size());
        vec.assign(m_pendingCompletions.begin(), m_pendingCompletions.end());

        this->syncCompletions(vec, silent);
    }
}

void ALLManager::syncAllCompletions() {
    if (!this->isLoggedIn()) {
        return;
    }

    m_pendingCompletions.clear();
    m_registeredCompletions.clear();

    auto completions = std::unordered_set<int64_t>{};

    for (auto level : getCompletedLevels()) {
        auto id = level->m_levelID.value();

        if (id > 0) {
            completions.insert(id);
        }
    }

    auto completionsVec = std::vector<int64_t>{};
    completionsVec.reserve(completions.size());
    completionsVec.assign(completions.begin(), completions.end());

    this->syncCompletions(std::move(completionsVec), false);
}

void ALLManager::syncCompletions(std::vector<int64_t> completions, bool silent) {
    if (m_isSyncingCompletions || completions.empty() || !this->isLoggedIn()) {
        return;
    }

    auto total = completions.size();
    auto chunkSize = 5000;

    if (completions.size() > chunkSize) {
        m_pendingCompletions.insert(
            completions.begin() + chunkSize,
            completions.end()
        );

        completions.resize(chunkSize);
    }

    auto body = matjson::Value{};
    auto arr = matjson::Value::array();

    for (auto id : completions) {
        auto obj = matjson::Value{};

        obj["id"] = id;

        arr.push(obj);
    }

    body["completions"] = arr;
    
    m_isSyncingCompletions = true;

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(body)
            .post(fmt::format("{}/api/levels/bulk-complete", getBaseURL())),
        [this, silent, total, completions = std::move(completions)](web::WebResponse resp) {
            m_isSyncingCompletions = false;

            const auto onFailure = [this, silent](int failedCount) {
                for (int i = 0; i < m_syncCallbacks.size(); i++) {
                    m_syncCallbacks[i]();
                }

                m_syncCallbacks.clear();

                if (!silent) {
                    Notification::create(
                        fmt::format("Couldn't sync {} completion{}", failedCount, failedCount > 1 ? "s" : ""),
                        NotificationIcon::Error
                    )->show();
                }
            };

            auto code = resp.code();

            if (code == 401) {
                this->logout();
                
                if (!silent) {
                    Notification::create("Invalid session, please log in again", NotificationIcon::Error)->show();
                } else {
                    m_sessionExpired = true;
                }

                for (int i = 0; i < m_syncCallbacks.size(); i++) {
                    m_syncCallbacks[i]();
                }

                m_syncCallbacks.clear();

                return;
            }

            auto res = resp.json();

            if (!res.isOk()) {
                onFailure(total);
                return;
            }

            auto json = res.unwrap();

            if (!json["ok"].asBool().unwrapOr(false)) {
                onFailure(total);
                return;
            }
            
            auto done = std::unordered_set<int64_t>{};

            const auto checkID = [&, this](int64_t id) {
                if (id <= 0) {
                    return;
                }

                done.insert(id);
                m_registeredCompletions.insert(id);
            };

            for (const auto& v : json["queued"]) {
                checkID(v.asInt().unwrapOr(0));
            }

            for (const auto& v : json["errors"]) {
                checkID(v["id"].asInt().unwrapOr(0));
            }

            auto stillPending = std::unordered_set<int64_t>{};

            for (auto id : m_pendingCompletions) {
                if (!done.contains(id)) {
                    stillPending.insert(id);
                }
            }

            for (auto id : completions) {
                if (!done.contains(id)) {
                    stillPending.insert(id);
                }
            }

            m_pendingCompletions = std::move(stillPending);

            this->saveCompletions();

            if (!m_pendingCompletions.empty() && !done.empty()) {
                this->syncPendingCompletions(silent);
            } else if (!m_pendingCompletions.empty() && done.empty()) {
                onFailure(m_pendingCompletions.size());
            } else {
                for (int i = 0; i < m_syncCallbacks.size(); i++) {
                    m_syncCallbacks[i]();
                }

                m_syncCallbacks.clear();
            }
        }
    );
}

void ALLManager::savePendingCompletions() {
    auto arr = matjson::Value::array();

    for (auto id : m_pendingCompletions) {
        arr.push(id);
    }

    Mod::get()->setSavedValue("pending-completions", arr);
}

void ALLManager::saveRegisteredCompletions() {
    auto arr = matjson::Value::array();
    
    for (auto id : m_registeredCompletions) {
        arr.push(id);
    }

    Mod::get()->setSavedValue("registered-completions", arr);
}

void ALLManager::saveCompletions() {
    this->savePendingCompletions();
    this->saveRegisteredCompletions();
}

void ALLManager::addCompletion(int64_t id, bool ignoreAutoSync) {
    if (!ignoreAutoSync && this->isLoggedIn() && Mod::get()->getSavedValue<bool>("auto-sync-completions")) {
        this->tryCompleteLevel(id);
        return;
    }

    m_pendingCompletions.insert(id);
    this->savePendingCompletions();
}

bool ALLManager::isSyncingCompletions() {
    return m_isSyncingCompletions;
}

void ALLManager::listenForSync(SyncCallback callback) {
    if (m_isSyncingCompletions) {
        m_syncCallbacks.push_back(std::move(callback));
    }
}

void ALLManager::tryCompleteLevel(int64_t id) {
    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(
                matjson::makeObject({
                    { "completed", true }
                })
            )
            .post(fmt::format("{}/api/levels/{}/ratings", getBaseURL(), id)),
        [this, id](web::WebResponse resp) {
            auto code = resp.code();

            if (code == 401) {
                this->logout();
                m_sessionExpired = true;
                log::error("Failed to submit completion, HTTP {}", code);
                this->addCompletion(id, true);
                return;
            }

            auto res = resp.json();

            if (!res.isOk()) {
                log::error("Failed to submit completion, HTTP {}", code);
                this->addCompletion(id, true);
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                log::error("Failed to submit completion, HTTP {}, {}", code, json["error"].asString().unwrapOr("2"));
                this->addCompletion(id, true);
                return;
            }

            m_registeredCompletions.insert(id);
            this->saveRegisteredCompletions();
        }
    );
}

std::optional<LevelRating> ALLManager::levelRatingForLevel(int64_t id) {
    if (m_cachedLevelRatings.contains(id)) {
        return m_cachedLevelRatings.at(id);
    }

    return {};
}

void ALLManager::cacheLevelRating(int64_t id, LevelRating rating) {
    m_cachedLevelRatings[id] = std::move(rating);
}

void ALLManager::uncacheLevelRating(int64_t id) {
    m_cachedLevelRatings.erase(id);
}

void ALLManager::requestLevelRating(int64_t id, LevelRatingCallback callback) {
    if (!this->isLoggedIn()) {
        return;
    }
    
    if (m_levelRatingCallbacks.contains(id)) {
        m_levelRatingCallbacks.at(id).push_back(std::move(callback));
        return;
    }

    m_levelRatingCallbacks[id].push_back(std::move(callback));

    async::spawn(
        web::WebRequest()
            .get(fmt::format("{}/api/levels/{}/ratings", getBaseURL(), id)),
        [this, id](web::WebResponse resp) {
            const auto onFailure = [this, id](std::string_view err = "") {
                for (int i = 0; i < m_levelRatingCallbacks.at(id).size(); i++) {
                    m_levelRatingCallbacks.at(id)[i](Err(err));
                }

                m_levelRatingCallbacks.erase(id);
            };
            
            auto code = resp.code();

            auto res = resp.json();

            if (!res.isOk()) {
                onFailure();
                log::error("Failed to get level ratings, HTTP {}", code);
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                onFailure(json["error"].asString().unwrapOr(""));
                log::error("Failed to get level ratings, HTTP {}", code);
                return;
            }

            if (code != 200 && code != 201) {
                onFailure();
                log::error("Failed to get level ratings, HTTP {}", code);
                return;
            }

            const auto& user = this->getUser();
            auto rating = LevelRating{};

            for (const auto& v : json["ratings"]) {
                auto userId = v["userId"].asInt().unwrapOr(0);

                if (userId == user.id) {
                    rating = LevelRating{
                        .rating = static_cast<float>(v["enjoyment"].asDouble().unwrapOr(0.f)),
                        .rated = v.contains("enjoyment") && v["enjoyment"].asDouble().isOk(),
                        .review = v["review"].asString().unwrapOr("")
                    };

                    break;
                }
            }

            this->cacheLevelRating(id, rating);

            for (int i = 0; i < m_levelRatingCallbacks.at(id).size(); i++) {
                m_levelRatingCallbacks.at(id)[i](Ok(rating));
            }

            m_levelRatingCallbacks.erase(id);
        }
    );
}

void ALLManager::submitLevelRating(int64_t id, const LevelRating& rating, SubmitLevelRatingCallback callback) {
    if (m_submitRatingCallbacks.contains(id)) {
        return;
    }

    m_submitRatingCallbacks[id].push_back(std::move(callback));

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(
                matjson::makeObject({
                    { "enjoyment", rating.rated ? matjson::Value(rating.rating) : nullptr },
                    { "review", !rating.review.empty() ? matjson::Value(rating.review) : nullptr }
                })
            )
            .post(fmt::format("{}/api/levels/{}/ratings", getBaseURL(), id)),
        [this, id, rating](web::WebResponse resp) {
            const auto doCallbacks = [this, id](Result<> res) {
                for (int i = 0; i < m_submitRatingCallbacks.at(id).size(); i++) {
                    m_submitRatingCallbacks.at(id)[i](res);
                }

                m_submitRatingCallbacks.erase(id);
            };

            auto code = resp.code();

            if (code == 401) {
                this->logout();
                Notification::create("Invalid session, please log in again", NotificationIcon::Error)->show();
                log::error("Failed to submit rating, HTTP {}", code);
                doCallbacks(Err(""));
                return;
            }

            auto res = resp.json();

            if (!res.isOk()) {
                doCallbacks(Err(""));
                log::error("Failed to submit rating, HTTP {}", code);
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                doCallbacks(Err(json["error"].asString().unwrapOr("Failed to submit level rating")));
                log::error("Failed to submit rating, HTTP {}, {}", code, json["error"].asString().unwrapOr("2"));
                return;
            }

            if (code != 200 && code != 201) {
                doCallbacks(Err(""));
                log::error("Failed to submit rating, HTTP {}", code);
                return;
            }

            Notification::create("Level rating submitted", NotificationIcon::Success)->show();

            doCallbacks(Ok());

            this->cacheLevelRating(id, rating);
        }
    );
}

void ALLManager::listenForSubmitLevelRating(int64_t id, SubmitLevelRatingCallback callback) {
    if (m_submitRatingCallbacks.contains(id)) {
        m_submitRatingCallbacks.at(id).push_back(std::move(callback));
        return;
    }
}

bool ALLManager::isSubmittingRatingFor(int64_t id) {
    return m_submitRatingCallbacks.contains(id);
}

void ALLManager::getUserLists(UserListsCallback callback) {
    if (!this->isLoggedIn()) {
        return;
    }
    
    if (this->areUserListsCached()) {
        callback(Ok(m_userLists));
        return;
    }

    m_userListsCallbacks.push_back(std::move(callback));

    if (m_userListsCallbacks.size() > 1) {
        return;
    }

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .get(fmt::format("{}/api/users/{}/difficulty?lean=1", getBaseURL(), this->getUser().id)),
        [this](web::WebResponse resp) {
            const auto doCallbacks = [this](Result<const std::vector<UserList>&> res) {
                for (int i = 0; i < m_userListsCallbacks.size(); i++) {
                    m_userListsCallbacks[i](res);
                }

                m_userListsCallbacks.clear();
            };

            auto code = resp.code();

            if (code == 401) {
                this->logout();
                Notification::create("Invalid session, please log in again", NotificationIcon::Error)->show();
                doCallbacks(Err(""));
                return;
            }

            auto res = resp.json();

            if (!res.isOk()) {
                log::error("Failed to fetch user rankings, HTTP {}", code);
                doCallbacks(Err(""));
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                log::error("Failed to fetch user rankings, HTTP {}, {}", code, json["error"].asString().unwrapOr("unknown error"));
                doCallbacks(Err(""));
                return;
            }

            auto rankings = std::vector<LevelRanking>{};

            for (const auto& v : json["ranked"]) {
                rankings.push_back(LevelRanking{
                    .id = numFromString<int64_t>(v["levelId"].asString().unwrapOr("0")).unwrapOr(0),
                    .name = v["name"].asString().unwrapOr(""),
                    .listID = numFromString<int>(v["personalListId"].asString().unwrapOr("0")).unwrapOr(0),
                    .bucket = difficultyForString(v["bucket"].asString().unwrapOr(""))
                });
            }

            async::spawn(
                web::WebRequest()
                    .header("Authorization", fmt::format("Bearer {}", m_token))
                    .get(fmt::format("{}/api/users/{}/lists", getBaseURL(), this->getUser().id)),
                [this, doCallbacks, rankings](web::WebResponse resp) {
                    auto code = resp.code();

                    if (code == 401) {
                        this->logout();
                        Notification::create("Invalid session, please log in again", NotificationIcon::Error)->show();
                        doCallbacks(Err(""));
                        return;
                    }

                    auto res = resp.json();

                    if (!res.isOk()) {
                        log::error("Failed to fetch user lists, HTTP {}", code);
                        doCallbacks(Err(""));
                        return;
                    }

                    auto json = res.unwrap();

                    if (json.contains("error")) {
                        log::error("Failed to fetch user lists, HTTP {}, {}", code, json["error"].asString().unwrapOr("unknown error"));
                        doCallbacks(Err(""));
                        return;
                    }

                    m_userLists.clear();

                    m_userLists.push_back(UserList{
                        .name = "All",
                        .sortOrder = 1
                    });

                    m_userLists.push_back(UserList{
                        .name = "Main",
                        .sortOrder = 2
                    });

                    auto listIDs = std::unordered_set<int64_t>{};

                    for (const auto& v : json["lists"]) {
                        auto id = static_cast<int>(v["id"].asInt().unwrapOr(0));

                        if (id <= 0) {
                            continue;
                        }

                        m_userLists.push_back(UserList{
                            .id = id,
                            .name = v["name"].asString().unwrapOr(""),
                            .sortOrder = static_cast<int>(v["sortOrder"].asInt().unwrapOr(0))
                        });

                        listIDs.insert(id);
                    }

                    for (const auto& ranking : rankings) {
                        m_userLists[0].levels.push_back(ranking);

                        if (ranking.listID <= 0 || !listIDs.contains(ranking.listID)) {
                            m_userLists[1].levels.push_back(ranking);
                        } else {
                            for (auto& list : m_userLists) {
                                if (list.id == ranking.listID) {
                                    list.levels.push_back(ranking);
                                    break;
                                }
                            }
                        }
                    }
                    
                    doCallbacks(Ok(m_userLists));

                    m_didCacheUserLists = true;
                }
            );
        }
    );
}

bool ALLManager::areUserListsCached() {
    return m_didCacheUserLists;
}

void ALLManager::removeUserListsCache() {
    m_didCacheUserLists = false;
}

std::optional<std::unordered_map<Difficulty, std::vector<LevelRanking>>> ALLManager::getAnchorLevels() {
    if (!m_anchorLevels.empty()) {
        return m_anchorLevels;
    }

    return {};
}

void ALLManager::requestAnchorLevels(AnchorLevelsCallback callback) {
    if (!m_anchorLevelsCallbacks.empty()) {
        m_anchorLevelsCallbacks.push_back(std::move(callback));
        return;
    }

    m_anchorLevelsCallbacks.push_back(std::move(callback));

    async::spawn(
        web::WebRequest()
            .get(fmt::format("{}/api/levels/anchors", getBaseURL())),
        [this](web::WebResponse resp) {
            const auto doCallbacks = [this](Result<const std::unordered_map<Difficulty, std::vector<LevelRanking>>&> res) {
                for (int i = 0; i < m_anchorLevelsCallbacks.size(); i++) {
                    m_anchorLevelsCallbacks[i](res);
                }

                m_anchorLevelsCallbacks.clear();
            };

            auto code = resp.code();

            auto res = resp.json();

            if (!res.isOk()) {
                doCallbacks(Err(""));
                log::error("Failed to get level anchors, HTTP {}", code);
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                doCallbacks(Err(""));
                log::error("Failed to get level anchors, HTTP {}, {}", code, json["error"].asString().unwrapOr("unknown"));
                return;
            }

            m_anchorLevels.clear();

            auto levels = std::unordered_map<int64_t, LevelRanking>{};

            const auto scanLevels = [&](const matjson::Value& arr) {
                for (const auto& v : arr) {
                    auto id = numFromString<int64_t>(v["levelId"].asString().unwrapOr("0")).unwrapOr(0);

                    if (id <= 0 || levels.contains(id)) {
                        return;
                    }

                    levels[id] = LevelRanking{
                        .id = id,
                        .name = v["name"].asString().unwrapOr(""),
                        .bucket = difficultyForString(v["difficulty"].asString().unwrapOr(""))
                    };
                }
            };

            scanLevels(json["main"]);
            scanLevels(json["bot"]);

            for (const auto& v : json["bucketWalk"]) {
                auto key = v.getKey().value_or("");
                auto difficulty = difficultyForString(key);

                if (difficulty == Difficulty::Unknown) {
                    return;
                }

                for (const auto& b : v) {
                    auto id = numFromString<int64_t>(b.asString().unwrapOr("0")).unwrapOr(0);

                    if (levels.contains(id)) {
                        m_anchorLevels[difficulty].push_back(levels.at(id));
                    }
                }
            }

            doCallbacks(Ok(m_anchorLevels));
        }
    );
}

void ALLManager::submitDifficultyPlacements(const std::vector<LevelSubmitInfo>& infos, PlacementSubmitCallback callback) {
    if (!this->isLoggedIn()) {
        return;
    }

    auto json = matjson::Value{};
    auto order = matjson::Value::array();
    auto shelves = matjson::Value::array();
    auto refsAbove = matjson::Value::array();
    auto refsBelow = matjson::Value::array();
    auto lists = matjson::Value::array();

    for (const auto& info : infos) {
        order.push(info.id);
        shelves.push(info.bucket != Difficulty::Unknown ? matjson::Value(stringForDifficulty(info.bucket)) : matjson::Value(nullptr));
        refsAbove.push(info.refAbove != 0 ? matjson::Value(numToString(info.refAbove)) : matjson::Value(nullptr));
        refsBelow.push(info.refBelow != 0 ? matjson::Value(numToString(info.refBelow)) : matjson::Value(nullptr));
        lists.push(info.listID != 0 ? matjson::Value(info.listID) : matjson::Value(nullptr));
    }

    json["order"] = order;
    json["shelves"] = shelves;
    json["refsAbove"] = refsAbove;
    json["refsBelow"] = refsBelow;
    json["lists"] = lists;

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(json)
            .put(fmt::format("{}/api/users/{}/difficulty", getBaseURL(), this->getUser().id)),
        [this, callback = std::move(callback)](web::WebResponse resp) mutable {
            auto code = resp.code();

            if (code == 401) {
                this->logout();
                m_sessionExpired = true;
                log::error("Failed to submit difficulty placements, HTTP {}", code);
                callback(Err("Invalid session, please log in again"));
                return;
            }

            auto res = resp.json();

            if (!res.isOk()) {
                log::error("Failed to submit difficulty placements, HTTP {}", code);
                callback(Err("Failed to submit placement"));
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                auto res = json["error"].asString();
                log::error("Failed to submit difficulty placements, HTTP {}, {}", code, res.unwrapOr("unknown error"));
                callback(Err(res.isOk() ? res.unwrap() : "Failed to submit placement"));
                return;
            }

            callback(Ok());

            m_didCacheUserLists = false;
        }
    );
}

void ALLManager::trySendLevelData(GJGameLevel* level) {
    if (!level) {
        return;
    }

    auto id = level->m_levelID.value();

    if (id <= 0 || m_levelsDataSent.contains(id)) {
        return;
    }

    // todo possibly more than just 2 player

    if (!level->m_twoPlayerMode) {
        return;
    }
 
    auto json = matjson::Value{};

    json["levelId"] = id;
    json["twoPlayer"] = level->m_twoPlayerMode;

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(json)
            .put(fmt::format("{}/api/levels/report-2p", getBaseURL())),
        [this, id](web::WebResponse resp) {
            auto code = resp.code();

            if (code != 200) {
                if (code == 401) {
                    this->logout();
                    m_sessionExpired = true;
                }

                log::error("Failed to report level data, HTTP {}", code);

                return;
            }

            m_levelsDataSent.insert(id);

            auto arr = matjson::Value::array();

            for (auto id : m_levelsDataSent) {
                arr.push(id);
            }

            Mod::get()->setSavedValue("levels-data-sent", arr);
        }
    );
}

void ALLManager::isLevelInList(int64_t id, LevelInListCallback callback) {
    if (m_levelsInlist.contains(id)) {
        callback(Ok(m_levelsInlist.at(id)));
        return;
    }

    m_levelInListCallbacks[id].push_back(std::move(callback));

    if (m_levelInListCallbacks.at(id).size() > 1) {
        return;
    }

    async::spawn(
        web::WebRequest()
            .get(fmt::format("{}/levels/{}/points", getBaseURL(), id)),
        [this, id](web::WebResponse resp) {
            const auto doCallbacks = [this, id](Result<bool> res) {
                for (int i = 0; i < m_levelInListCallbacks.at(id).size(); i++) {
                    m_levelInListCallbacks.at(id)[i](res);
                }

                m_levelInListCallbacks.erase(id);
            };

            auto code = resp.code();
            auto res = resp.json();

            if (!res.isOk()) {
                doCallbacks(Err(""));
                log::error("Failed to see if {} is in the list, HTTP {}", id, code);
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                auto err = json["error"].asString().unwrapOr("unknown error");
                doCallbacks(Err(err));
                log::error("Failed to see if {} is in the list, {}, HTTP {}", id, err, code);
                return;
            }

            if (!json["inList"].asBool().isOk()) {
                doCallbacks(Err(""));
                log::error("Failed to see if {} is in the list, HTTP {}", id, code);
                return;
            }
            
            auto inList = json["inList"].asBool().unwrap();

            doCallbacks(Ok(inList));
            
            this->setLevelInList(id, inList);
            this->saveLevelsInList();
        }
    );
}

void ALLManager::setLevelInList(int64_t id, bool inList) {
    m_levelsInlist[id] = inList;
}

void ALLManager::saveLevelsInList() {
    auto arr = matjson::Value::array();

    for (const auto& [id, inList] : m_levelsInlist) {
        if (inList) {
            arr.push(id);
        }
    }

    Mod::get()->setSavedValue("levels-in-list", arr);
}

void ALLManager::tryAddLevel(int64_t id, GJGameLevel* level, AddLevelCallback callback) {
    if (!level || level->m_unlisted || !this->isLoggedIn()) {
        return;
    }

    if (m_levelsInlist.contains(id)) {
        callback(Ok(m_levelsInlist.at(id)));
        return;
    }

    m_addLevelCallbacks[id].push_back(std::move(callback));

    if (m_addLevelCallbacks.at(id).size() > 1) {
        return;
    }

    auto body = matjson::Value{};

    body["name"] = std::string(level->m_levelName);
    body["description"] = std::string(level->m_levelDesc);
    body["authorId"] = level->m_accountID.value();
    body["authorName"] = std::string(level->m_creatorName);

    if (difficultyForLevel(level) != Difficulty::Unknown) {
        body["difficulty"] = stringForDifficulty(difficultyForLevel(level));
    }
    
    body["stars"] = level->m_stars.value();
    body["requestedStars"] = level->m_starsRequested;
    body["downloads"] = level->m_downloads;
    body["likes"] = level->m_likes;
    body["length"] = stringForLength(level->m_levelLength);
    body["coins"] = level->m_coins;
    body["featured"] = level->m_featured >= 1 || level->m_isEpic > 0;
    body["epic"] = level->m_isEpic;
    body["officialSongId"] = level->m_audioTrack;
    body["customSongId"] = level->m_songID;

    async::spawn(
        web::WebRequest()
            .header("Authorization", fmt::format("Bearer {}", m_token))
            .bodyJSON(body)
            .post(fmt::format("{}/levels/{}", getBaseURL(), id)),
        [this, id](web::WebResponse resp) {
            const auto doCallbacks = [this, id](Result<bool> res) {
                if (!res.isOk()) {
                    this->setLevelInList(id, false);
                }

                for (int i = 0; i < m_addLevelCallbacks.at(id).size(); i++) {
                    m_addLevelCallbacks.at(id)[i](res);
                }

                m_addLevelCallbacks.erase(id);
            };

            auto code = resp.code();
            auto res = resp.json();

            if (!res.isOk()) {
                auto err = fmt::format("Failed to add level to list, HTTP {}", code);
                doCallbacks(Err(err));
                log::error("{}", err);
                return;
            }

            auto json = res.unwrap();

            if (json.contains("error")) {
                auto err = json["error"].asString().unwrapOr("unknown error");
                doCallbacks(Err(fmt::format("Failed to add level to list, {}", err)));
                log::error("Failed to add level to list, HTTP {}, {}", code, err);
                return;
            }

            if (code != 200 && code != 201) {
                auto err = fmt::format("Failed to add level to list, HTTP {}", code);
                doCallbacks(Err(err));
                log::error("{}", err);
                return;
            }

            this->setLevelInList(id, true);
            this->saveLevelsInList();

            doCallbacks(Ok(true));
        }
    );
}

bool ALLManager::isAddingLevel(int64_t id) {
    return m_addLevelCallbacks.contains(id);
}