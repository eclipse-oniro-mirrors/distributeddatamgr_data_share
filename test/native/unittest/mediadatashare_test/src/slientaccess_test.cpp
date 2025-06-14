/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include <unistd.h>
#include <vector>

#include "accesstoken_kit.h"
#include "data_ability_observer_stub.h"
#include "datashare_helper.h"
#include "datashare_log.h"
#include "hap_token_info.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

namespace OHOS {
namespace DataShare {
using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
using ChangeInfo = DataShareObserver::ChangeInfo;
constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
static int USER_100 = 100;
std::string DATA_SHARE_URI = "datashare:///com.acts.datasharetest";
std::string DATA_SHARE_ERROR_URI = "datashare:///com.acts.datasharetest000";
std::string SLIENT_ACCESS_URI = "datashare:///com.acts.datasharetest/entry/DB00/TBL00?Proxy=true";
std::string SLIENT_ERROR_URI = "datashare:///com.acts.datashare/entry/DB00/TBL00?Proxy=true";
std::string SLIENT_ERROR_DATABASE_URI = "datashare:///com.acts.datasharetest/entry/DB6666/TBL00?Proxy=true";
std::string SLIENT_REGISTER_URI = "datashare:///com.acts.datasharetest/entry/DB00/TBL02?Proxy=true";
std::string SLIENT_ACCESS_PERMISSION1_URI = "datashare:///com.acts.datasharetest/entry/DB00/permission1?Proxy=true";
std::string SLIENT_PROXY_PERMISSION1_URI = "datashareproxy://com.acts.datasharetest/entry/DB00/permission1";
std::string SLIENT_ACCESS_PERMISSION2_URI = "datashare:///com.acts.datasharetest/entry/DB00/permission2?Proxy=true";
std::string SLIENT_PROXY_PERMISSION2_URI = "datashareproxy://com.acts.datasharetest/entry/DB00/permission2";
std::string TBL_STU_NAME = "name";
std::string TBL_STU_AGE = "age";
std::shared_ptr<DataShare::DataShareHelper> g_slientAccessHelper;

template <typename T>
class ConditionLock {
public:
    explicit ConditionLock() {}
    ~ConditionLock() {}
public:
    void Notify()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isSet_ = true;
        cv_.notify_one();
    }
    
    void Wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::seconds(INTERVAL), [this]() { return isSet_; });
        cv_.notify_one();
        return;
    }
    
    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isSet_ = false;
        cv_.notify_one();
    }

private:
    bool isSet_ = false;
    T data_;
    std::mutex mutex_;
    std::condition_variable cv_;
    static constexpr int64_t INTERVAL = 2;
};

class SlientAccessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class IDataShareAbilityObserverTest : public AAFwk::DataAbilityObserverStub {
public:
    IDataShareAbilityObserverTest() =  default;

    ~IDataShareAbilityObserverTest()
    {}

    void OnChange()
    {
        name = "OnChangeName";
        data.Notify();
    }

    std::string GetName()
    {
        return name;
    }

    void SetName(std::string name)
    {
        this->name = name;
    }
	
    void Clear()
    {
        data.Clear();
    }
    ConditionLock<std::string> data;
private:
    std::string name;
};

class DataShareObserverTest : public DataShare::DataShareObserver {
public:
    DataShareObserverTest() {}
    ~DataShareObserverTest() {}
    
    void OnChange(const ChangeInfo &changeInfo) override
    {
        changeInfo_ = changeInfo;
        data.Notify();
    }
    
    void Clear()
    {
        changeInfo_.changeType_ = INVAILD;
        changeInfo_.uris_.clear();
        changeInfo_.data_ = nullptr;
        changeInfo_.size_ = 0;
        changeInfo_.valueBuckets_ = {};
        data.Clear();
    }
    
    ChangeInfo changeInfo_;
    ConditionLock<ChangeInfo> data;
};

std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(int32_t systemAbilityId, std::string uri)
{
    LOG_INFO("CreateDataShareHelper start");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, uri);
}

std::vector<PermissionStateFull> GetPermissionStateFulls()
{
    std::vector<PermissionStateFull> permissionStateFulls = {
        {
            .permissionName = "ohos.permission.WRITE_CONTACTS",
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 }
        },
        {
            .permissionName = "ohos.permission.WRITE_CALL_LOG",
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 }
        },
        {
            .permissionName = "ohos.permission.GET_BUNDLE_INFO",
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 }
        }
    };
    return permissionStateFulls;
}

void SlientAccessTest::SetUpTestCase(void)
{
    LOG_INFO("SetUpTestCase invoked");
    auto dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID, DATA_SHARE_URI);
    ASSERT_TRUE(dataShareHelper != nullptr);
    int sleepTime = 3;
    sleep(sleepTime);

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto permStateList = GetPermissionStateFulls();
    HapPolicyParams policy = {
        .apl = APL_NORMAL,
        .domain = "test.domain",
        .permList = {
            {
                .permissionName = "ohos.permission.test",
                .bundleName = "ohos.datashareclienttest.demo",
                .grantMode = 1,
                .availableLevel = APL_NORMAL,
                .label = "label",
                .labelId = 1,
                .description = "ohos.datashareclienttest.demo",
                .descriptionId = 1
            }
        },
        .permStateList = permStateList
    };
    AccessTokenKit::AllocHapToken(info, policy);
    auto testTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenIDEx(
        info.userID, info.bundleName, info.instIndex);
    SetSelfTokenID(testTokenId.tokenIDEx);

    g_slientAccessHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID, SLIENT_ACCESS_URI);
    ASSERT_TRUE(g_slientAccessHelper != nullptr);
    LOG_INFO("SetUpTestCase end");
}

void SlientAccessTest::TearDownTestCase(void)
{
    auto tokenId = AccessTokenKit::GetHapTokenIDEx(100, "ohos.datashareclienttest.demo", 0);
    AccessTokenKit::DeleteToken(tokenId.tokenIDEx);
    g_slientAccessHelper = nullptr;
}

void SlientAccessTest::SetUp(void) {}
void SlientAccessTest::TearDown(void) {}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_Errorcode_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_001::Start");
    std::string uriStr1(SLIENT_ACCESS_URI);
    std::string uriStr2 (DATA_SHARE_URI);
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj, uriStr1, uriStr2);
    EXPECT_EQ(ret, DataShare::E_OK);
    EXPECT_NE(helper, nullptr);
    helper = nullptr;
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_Errorcode_Test_002, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_002::Start");
    std::string uriStr1(SLIENT_ACCESS_URI);
    std::string uriStr2 (DATA_SHARE_URI);
    auto [ret, helper] = DataShare::DataShareHelper::Create(nullptr, uriStr1, uriStr2);
    EXPECT_EQ(ret, DataShare::E_TOKEN_EMPTY);
    EXPECT_EQ(helper, nullptr);
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_Errorcode_Test_003, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_003::Start");
    std::string uriStr1(SLIENT_ERROR_URI);
    std::string uriStr2 ("");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    // slientUri is error bundle name, extUri is empty, slient access can't find the bundle name
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj, uriStr1, uriStr2);
    EXPECT_EQ(ret, DataShare::E_EXT_URI_INVALID);
    EXPECT_EQ(helper, nullptr);
    helper = nullptr;
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_003::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_Errorcode_Test_004, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_004::Start");
    std::string uriStr1(DATA_SHARE_URI);
    std::string uriStr2 (DATA_SHARE_URI);
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj, uriStr1, uriStr2);
    EXPECT_EQ(ret, DataShare::E_OK);
    EXPECT_NE(helper, nullptr);
    helper = nullptr;
    LOG_INFO("SlientAccess_Creator_Errorcode_Test_004::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_InsertEx_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_InsertEx_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    auto [errCode, retVal] = helper->InsertEx(uri, valuesBucket);
    EXPECT_EQ((errCode == 0), true);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_InsertEx_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_InsertEx_Test_002, TestSize.Level0)
{
    LOG_INFO("SilentAccess_InsertEx_Test_002::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    EXPECT_EQ(helper->DataShareHelper::InsertEx(uri, valuesBucket), std::make_pair(0, 0));
    LOG_INFO("SilentAccess_InsertEx_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_UpdateEx_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_UpdateEx_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    int value = 50;
    valuesBucket.Put(TBL_STU_AGE, value);
    DataShare::DataSharePredicates predicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    predicates.SetWhereClause(selections);
    auto [errCode, retVal] = helper->UpdateEx(uri, predicates, valuesBucket);
    EXPECT_EQ((errCode == 0), true);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_UpdateEx_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_UpdateEx_Test_002, TestSize.Level0)
{
    LOG_INFO("SilentAccess_UpdateEx_Test_002::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    int value = 50;
    valuesBucket.Put(TBL_STU_AGE, value);
    DataShare::DataSharePredicates predicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    predicates.SetWhereClause(selections);

    EXPECT_EQ(helper->DataShareHelper::UpdateEx(uri, predicates, valuesBucket), std::make_pair(0, 0));
    LOG_INFO("SilentAccess_UpdateEx_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_DeleteEx_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_DeleteEx_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);

    DataShare::DataSharePredicates deletePredicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    deletePredicates.SetWhereClause(selections);
    auto [errCode, retVal] = helper->DeleteEx(uri, deletePredicates);
    EXPECT_EQ((errCode == 0), true);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_DeleteEx_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_DeleteEx_Test_002, TestSize.Level0)
{
    LOG_INFO("SilentAccess_DeleteEx_Test_002::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataSharePredicates deletePredicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    deletePredicates.SetWhereClause(selections);

    EXPECT_EQ(helper->DataShareHelper::DeleteEx(uri, deletePredicates), std::make_pair(0, 0));
    LOG_INFO("SilentAccess_DeleteEx_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Insert_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Insert_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_Insert_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Update_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Update_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    int value = 50;
    valuesBucket.Put(TBL_STU_AGE, value);
    DataShare::DataSharePredicates predicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    predicates.SetWhereClause(selections);
    int retVal = helper->Update(uri, predicates, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_Update_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Query_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Query_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(TBL_STU_NAME, "lisi");
    vector<string> columns;
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("SlientAccess_Query_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Delete_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Delete_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    
    DataShare::DataSharePredicates deletePredicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    deletePredicates.SetWhereClause(selections);
    int retVal = helper->Delete(uri, deletePredicates);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_Delete_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Register_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Register_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    sptr<IDataShareAbilityObserverTest> dataObserver(new (std::nothrow) IDataShareAbilityObserverTest());
    dataObserver->SetName("zhangsan");
    helper->RegisterObserver(uri, dataObserver);
    EXPECT_EQ(dataObserver->GetName(), "zhangsan");
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);
    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    dataObserver->data.Wait();
    EXPECT_EQ(dataObserver->GetName(), "OnChangeName");
    dataObserver->Clear();

    DataShare::DataSharePredicates deletePredicates;
    deletePredicates.EqualTo(TBL_STU_NAME, "lisi")->And()->EqualTo(TBL_STU_NAME, 25);
    retVal = helper->Delete(uri, deletePredicates);
    EXPECT_EQ((retVal >= 0), true);
    helper->UnregisterObserver(uri, dataObserver);
    LOG_INFO("SlientAccess_Register_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_RegisterErrorUri_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_RegisterErrorUri_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    sptr<IDataShareAbilityObserverTest> dataObserver(new (std::nothrow) IDataShareAbilityObserverTest());
    dataObserver->SetName("zhangsan");
    Uri uriRegister(SLIENT_REGISTER_URI);
    helper->RegisterObserver(uriRegister, dataObserver);
    EXPECT_EQ(dataObserver->GetName(), "zhangsan");
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);
    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    EXPECT_NE(dataObserver->GetName(), "OnChangeName");

    DataShare::DataSharePredicates deletePredicates;
    deletePredicates.EqualTo(TBL_STU_NAME, "lisi")->And()->EqualTo(TBL_STU_NAME, 25);
    retVal = helper->Delete(uri, deletePredicates);
    EXPECT_EQ((retVal >= 0), true);
    helper->UnregisterObserver(uriRegister, dataObserver);
    LOG_INFO("SlientAccess_RegisterErrorUri_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_NoRegister_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_NoRegister_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    sptr<IDataShareAbilityObserverTest> dataObserver(new (std::nothrow) IDataShareAbilityObserverTest());
    dataObserver->SetName("zhangsan");
    EXPECT_EQ(dataObserver->GetName(), "zhangsan");
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);
    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    EXPECT_NE(dataObserver->GetName(), "OnChangeName");

    DataShare::DataSharePredicates deletePredicates;
    deletePredicates.EqualTo(TBL_STU_NAME, "lisi")->And()->EqualTo(TBL_STU_NAME, 25);
    retVal = helper->Delete(uri, deletePredicates);
    EXPECT_EQ((retVal >= 0), true);
    helper->UnregisterObserver(uri, dataObserver);
    LOG_INFO("SlientAccess_NoRegister_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_NoRegister_Test_002, TestSize.Level0)
{
    LOG_INFO("SlientAccess_NoRegister_Test_002::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    sptr<IDataShareAbilityObserverTest> dataObserver(new (std::nothrow) IDataShareAbilityObserverTest());
    dataObserver->SetName("zhangsan");
    helper->RegisterObserver(uri, dataObserver);
    helper->UnregisterObserver(uri, dataObserver);
    EXPECT_EQ(dataObserver->GetName(), "zhangsan");
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);
    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    EXPECT_NE(dataObserver->GetName(), "OnChangeName");

    DataShare::DataSharePredicates deletePredicates;
    deletePredicates.EqualTo(TBL_STU_NAME, "lisi")->And()->EqualTo(TBL_STU_NAME, 25);
    retVal = helper->Delete(uri, deletePredicates);
    EXPECT_EQ((retVal >= 0), true);
    LOG_INFO("SlientAccess_NoRegister_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Insert_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Insert_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ACCESS_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_Permission_Insert_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Insert_Test_003, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Insert_Test_003::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_PROXY_PERMISSION1_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_Permission_Insert_Test_003::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Update_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Update_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_PROXY_PERMISSION1_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    int value = 50;
    valuesBucket.Put(TBL_STU_AGE, value);
    DataShare::DataSharePredicates predicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    predicates.SetWhereClause(selections);
    int retVal = helper->Update(uri, predicates, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SlientAccess_Permission_Update_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Query_Test_002, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Query_Test_002::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_PROXY_PERMISSION2_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);

    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(TBL_STU_NAME, "lisi");
    vector<string> columns;
    DatashareBusinessError businessError;
    auto resultSet = helper->Query(uri, predicates, columns, &businessError);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    EXPECT_EQ(businessError.GetCode(), 0);
    LOG_INFO("SlientAccess_Permission_Query_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Delete_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Delete_Test_001::Start");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_PROXY_PERMISSION2_URI);
    
    DataShare::DataSharePredicates deletePredicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    deletePredicates.SetWhereClause(selections);
    int retVal = helper->Delete(uri, deletePredicates);
    EXPECT_EQ(retVal, 1);
    LOG_INFO("SlientAccess_Permission_Delete_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Insert_Test_002, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Insert_Test_002::Start");
    HapInfoParams info = {
        .userID = USER_100,
        .bundleName = "ohos.permission.write.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .apiVersion = 8,
        .appIDDesc = "ohos.permission.write.demo"
    };
    HapPolicyParams policy = {
        .apl = APL_SYSTEM_CORE,
        .domain = "test.domain",
        .permStateList = {
            {
                .permissionName = "ohos.permission.WRITE_CONTACTS",
                .isGeneral = true,
                .resDeviceID = { "local" },
                .grantStatus = { PermissionState::PERMISSION_GRANTED },
                .grantFlags = { 1 }
            }
        }
    };
    AccessTokenKit::AllocHapToken(info, policy);
    auto testTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenIDEx(
        info.userID, info.bundleName, info.instIndex);
    SetSelfTokenID(testTokenId.tokenIDEx);

    auto helper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID, SLIENT_ACCESS_URI);
    Uri uri(SLIENT_ACCESS_PERMISSION1_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);
    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ(retVal, -2);
    helper = nullptr;
    AccessTokenKit::DeleteToken(testTokenId.tokenIDEx);
    LOG_INFO("SlientAccess_Permission_Insert_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Update_Test_002, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Update_Test_002::Start");
    HapInfoParams info = {
        .userID = USER_100,
        .bundleName = "ohos.permission.write.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .apiVersion = 8,
        .appIDDesc = "ohos.permission.write.demo"
    };
    HapPolicyParams policy = {
        .apl = APL_SYSTEM_CORE,
        .domain = "test.domain",
        .permStateList = {
            {
                .permissionName = "ohos.permission.WRITE_CONTACTS",
                .isGeneral = true,
                .resDeviceID = { "local" },
                .grantStatus = { PermissionState::PERMISSION_GRANTED },
                .grantFlags = { 1 }
            }
        }
    };
    AccessTokenKit::AllocHapToken(info, policy);
    auto testTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenIDEx(
        info.userID, info.bundleName, info.instIndex);
    SetSelfTokenID(testTokenId.tokenIDEx);

    auto helper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID, SLIENT_ACCESS_URI);
    Uri uri(SLIENT_ACCESS_PERMISSION1_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    int value = 50;
    valuesBucket.Put(TBL_STU_AGE, value);
    DataShare::DataSharePredicates predicates;
    std::string selections = TBL_STU_NAME + " = 'lisi'";
    predicates.SetWhereClause(selections);
    int retVal = helper->Update(uri, predicates, valuesBucket);
    EXPECT_EQ(retVal, -2);
    helper = nullptr;
    AccessTokenKit::DeleteToken(testTokenId.tokenIDEx);
    LOG_INFO("SlientAccess_Permission_Update_Test_002::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Permission_Query_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Query_Test_001::Start");
    HapInfoParams info = {
        .userID = USER_100,
        .bundleName = "ohos.permission.write.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .apiVersion = 8,
        .appIDDesc = "ohos.permission.write.demo"
    };
    HapPolicyParams policy = {
        .apl = APL_SYSTEM_CORE,
        .domain = "test.domain",
        .permStateList = {
            {
                .permissionName = "ohos.permission.WRITE_CONTACTS",
                .isGeneral = true,
                .resDeviceID = { "local" },
                .grantStatus = { PermissionState::PERMISSION_GRANTED },
                .grantFlags = { 1 }
            }
        }
    };
    AccessTokenKit::AllocHapToken(info, policy);
    auto testTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenIDEx(
        info.userID, info.bundleName, info.instIndex);
    SetSelfTokenID(testTokenId.tokenIDEx);

    auto helper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID, SLIENT_ACCESS_URI);
    Uri uri(SLIENT_ACCESS_PERMISSION2_URI);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(TBL_STU_NAME, "lisi");
    vector<string> columns;
    DatashareBusinessError businessError;
    auto resultSet = helper->Query(uri, predicates, columns, &businessError);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 0);
    EXPECT_EQ(resultSet, nullptr);
    EXPECT_EQ(businessError.GetCode(), -2);
    helper = nullptr;
    AccessTokenKit::DeleteToken(testTokenId.tokenIDEx);
    LOG_INFO("SlientAccess_Permission_Query_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Access_When_Uri_Error_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Permission_Access_When_URI_ERROR_Test_001::Begin");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ERROR_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal < 0), true);
    LOG_INFO("SlientAccess_Permission_Access_When_URI_ERROR_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Access_With_Uncreated_DataBase_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Access_With_Uncreated_DataBase_Test_001::Begin");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ERROR_DATABASE_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    std::string value = "lisi";
    valuesBucket.Put(TBL_STU_NAME, value);
    int age = 25;
    valuesBucket.Put(TBL_STU_AGE, age);

    int retVal = helper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal < 0), true);
    LOG_INFO("SlientAccess_Access_With_Uncreated_DataBase_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_With_Uri_Error_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_With_Uri_Error_Test_001::Begin");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    std::string uriStr(DATA_SHARE_ERROR_URI);
    auto helper = DataShare::DataShareHelper::Creator(remoteObj, uriStr, uriStr, 2);
    EXPECT_EQ(helper, nullptr);
    LOG_INFO("SlientAccess_Creator_With_Uri_Error_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_When_TimeOut_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_With_Uri_Error_Test_001::Begin");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    std::string uriStr(DATA_SHARE_URI);
    auto helper = DataShare::DataShareHelper::Creator(remoteObj, uriStr, uriStr, 0);
    EXPECT_EQ(helper, nullptr);
    LOG_INFO("SlientAccess_Creator_With_Uri_Error_Test_001::End");
}

/*
* @tc.desc: test UserDefineFunc
* @tc.require: Null
*/
HWTEST_F(SlientAccessTest, SlientAccess_UserDefineFunc_Test_001, TestSize.Level0)
{
    LOG_INFO("SilentAccess_UserDefineFunc_Test_001::Start");
    auto helper = g_slientAccessHelper;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto result = helper->DataShareHelper::UserDefineFunc(data, reply, option);
    EXPECT_EQ(result, 0);
    LOG_INFO("SilentAccess_UserDefineFunc_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_ErrorBundle_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_ErrorBundle_Test_001::Start");
    std::string uriStr1("datashareproxy://com.acts.error.bundleName/test");
    // slientUri is error bundle name, slient access can't find the bundle name, return nullptr
    auto helperSilent = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID, uriStr1);
    EXPECT_EQ(helperSilent, nullptr);
    LOG_INFO("SlientAccess_Creator_ErrorBundle_Test_001::End");
}

HWTEST_F(SlientAccessTest, SlientAccess_Creator_ErrorBundle_ExtSuccess_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Creator_ErrorBundle_ExtSuccess_Test_001::Start");
    std::string uriStr1(SLIENT_ERROR_URI);
    std::string uriStr2 (DATA_SHARE_URI);
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    // slientUri is error bundleName, extUri is effective, slient access can't find the bundleName, but ext success
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj, uriStr1, uriStr2);
    EXPECT_EQ(ret, DataShare::E_OK);
    EXPECT_NE(helper, nullptr);
    helper = nullptr;
    LOG_INFO("SlientAccess_Creator_ErrorBundle_ExtSuccess_Test_001::End");
}

 /**
* @tc.name: SlientAccess_Create_With_Invalid_AppIndex_Test_001
* @tc.desc: test use invalid appIndex to create datashare helper
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SlientAccessTest, SlientAccess_Create_With_Invalid_AppIndex_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_Create_With_Invalid_AppIndex_Test_001::Start");
    std::string uri("datashareproxy://com.acts.datasharetest/test?Proxy=true&appIndex=-1");
    std::string extUri(DATA_SHARE_URI);
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("GetSystemAbilityManager get samgr failed.");
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        LOG_ERROR("GetSystemAbility service failed.");
    }
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj, uri, "");
    EXPECT_EQ(ret, E_EXT_URI_INVALID);
    EXPECT_EQ(helper, nullptr);

    helper = DataShare::DataShareHelper::Creator(remoteObj, uri);
    EXPECT_EQ(helper, nullptr);

    CreateOptions options;
    options.isProxy_ = true;
    helper = DataShare::DataShareHelper::Creator(uri, options);
    EXPECT_EQ(helper, nullptr);
    LOG_INFO("SlientAccess_Create_With_Invalid_AppIndex_Test_001::End");
}

/**
* @tc.name: SlientAccess_RegisterObserverExtProvider_Test_001
* @tc.desc: Fill the branch generalCtl == nullptr
* @tc.type: FUNC
*/
HWTEST_F(SlientAccessTest, SlientAccess_RegisterObserverExtProvider_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_RegisterObserverExtProvider_Test_001::Begin");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ERROR_URI);
    std::shared_ptr<DataShareObserver> dataObserver = std::make_shared<DataShareObserverTest>();
    ASSERT_NE(helper, nullptr);
    ASSERT_NE(dataObserver, nullptr);

    helper->RegisterObserverExtProvider(uri, dataObserver, false);

    LOG_INFO("SlientAccess_RegisterObserverExtProvider_Test_001::End");
}

/**
* @tc.name: SlientAccess_UnregisterObserverExtProvider_Test_001
* @tc.desc: Fill the branch generalCtl == nullptr
* @tc.type: FUNC
*/
HWTEST_F(SlientAccessTest, SlientAccess_UnregisterObserverExtProvider_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_UnregisterObserverExtProvider_Test_001::Begin");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ERROR_URI);
    std::shared_ptr<DataShareObserver> dataObserver = std::make_shared<DataShareObserverTest>();
    ASSERT_NE(helper, nullptr);
    ASSERT_NE(dataObserver, nullptr);

    helper->UnregisterObserverExtProvider(uri, dataObserver);

    ChangeInfo changeInfo = { DataShareObserver::ChangeType::INSERT, { uri } };
    LOG_INFO("SlientAccess_UnregisterObserverExtProvider_Test_001::End");
}

/**
* @tc.name: SlientAccess_NotifyChangeExtProvider_Test_001
* @tc.desc: Fill the branch generalCtl == nullptr
* @tc.type: FUNC
*/
HWTEST_F(SlientAccessTest, SlientAccess_NotifyChangeExtProvider_Test_001, TestSize.Level0)
{
    LOG_INFO("SlientAccess_NotifyChangeExtProvider_Test_001::Begin");
    auto helper = g_slientAccessHelper;
    Uri uri(SLIENT_ERROR_URI);
    std::shared_ptr<DataShareObserver> dataObserver = std::make_shared<DataShareObserverTest>();
    ASSERT_NE(helper, nullptr);
    ASSERT_NE(dataObserver, nullptr);

    helper->RegisterObserverExtProvider(uri, dataObserver, true);

    ChangeInfo changeInfo = { DataShareObserver::ChangeType::INSERT, { uri } };
    helper->NotifyChangeExtProvider(changeInfo);

    LOG_INFO("SlientAccess_NotifyChangeExtProvider_Test_001::End");
}
} // namespace DataShare
} // namespace OHOS