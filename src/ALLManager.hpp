#pragma once

#include "Includes.hpp"

using LoginCallback = Function<void(Result<>)>;
using SyncCallback = Function<void()>;
using LevelRatingCallback = Function<void(Result<const LevelRating&>)>;
using SubmitLevelRatingCallback = Function<void(Result<>)>;
using UserListsCallback = Function<void(Result<const std::vector<UserList>&>)>;
using AnchorLevelsCallback = Function<void(Result<const std::unordered_map<Difficulty, std::vector<LevelRanking>>&>)>;
using PlacementSubmitCallback = Function<void(Result<>)>;
using LevelInListCallback = Function<void(Result<bool>)>;
using AddLevelCallback = Function<void(Result<bool>)>;

struct LevelSubmitInfo {
    int id;
    Difficulty bucket;
    int refAbove;
    int refBelow;
    int listID;
};

class ALLManager {

private:

    bool m_isLoggingIn = false;
    std::vector<LoginCallback> m_loginCallbacks;

    std::string m_token;
    UserInfo m_user;

    int m_linkedGDAccountID = 0;
    std::string m_linkedGDAccountUsername;

    bool m_isSyncingCompletions = false;
    std::vector<SyncCallback> m_syncCallbacks;

    std::unordered_map<int, LevelRating> m_cachedLevelRatings;
    std::unordered_map<int, std::vector<LevelRatingCallback>> m_levelRatingCallbacks;

    std::unordered_map<int, std::vector<SubmitLevelRatingCallback>> m_submitRatingCallbacks;

    std::vector<UserList> m_userLists;
    std::vector<UserListsCallback> m_userListsCallbacks;
    bool m_didCacheUserLists = false;

    std::unordered_map<Difficulty, std::vector<LevelRanking>> m_anchorLevels;
    std::vector<AnchorLevelsCallback> m_anchorLevelsCallbacks;

    std::unordered_map<int, bool> m_levelsInlist;
    std::unordered_map<int, std::vector<LevelInListCallback>> m_levelInListCallbacks;

    std::unordered_map<int, std::vector<AddLevelCallback>> m_addLevelCallbacks;

public:

    std::unordered_set<int> m_pendingCompletions;
    std::unordered_set<int> m_registeredCompletions;

    bool m_sessionExpired = false;

    std::unordered_set<int> m_levelsDataSent;

    static ALLManager& get();
    
    void login(std::string_view, std::string_view, LoginCallback);
    bool isLoggingIn();
    void listenForLogin(LoginCallback);

    bool isLoggedIn();
    void setToken(std::string);
    void setUser(UserInfo);
    UserInfo getUser();
    bool isGDAccountLinked();
    void linkGDAccount();
    void setLinkedGDAccount(int, std::string = "");

    void logout();

    void updateUserInfo();

    void syncPendingCompletions(bool);
    void syncAllCompletions();
    void syncCompletions(std::vector<int>, bool);
    void savePendingCompletions();
    void saveRegisteredCompletions();
    void saveCompletions();
    void addCompletion(int, bool = false);

    bool isSyncingCompletions();
    void listenForSync(SyncCallback);

    void tryCompleteLevel(int);
    
    std::optional<LevelRating> levelRatingForLevel(int);
    void cacheLevelRating(int, LevelRating);
    void uncacheLevelRating(int);
    void requestLevelRating(int, LevelRatingCallback);
    void submitLevelRating(int, const LevelRating&, SubmitLevelRatingCallback);
    void listenForSubmitLevelRating(int, SubmitLevelRatingCallback);
    bool isSubmittingRatingFor(int);

    void getUserLists(UserListsCallback);
    bool areUserListsCached();
    void removeUserListsCache();

    std::optional<std::unordered_map<Difficulty, std::vector<LevelRanking>>> getAnchorLevels();
    void requestAnchorLevels(AnchorLevelsCallback);

    void submitDifficultyPlacements(const std::vector<LevelSubmitInfo>&, PlacementSubmitCallback);

    void trySendLevelData(GJGameLevel*);

    void isLevelInList(int, LevelInListCallback);
    void setLevelInList(int, bool);
    void saveLevelsInList();

    void tryAddLevel(int, GJGameLevel*, AddLevelCallback);
    bool isAddingLevel(int);

};