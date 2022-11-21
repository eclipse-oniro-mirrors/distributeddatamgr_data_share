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
#define MLOG_TAG "DataShareUnitTest"

#include "mediadatashare_unit_test.h"

#include "datashare_helper.h"
#include "fetch_result.h"
#include "get_self_permissions.h"
#include "iservice_registry.h"
#include "datashare_log.h"
#include "media_file_utils.h"
#include "media_library_manager.h"
#include "medialibrary_errno.h"
#include "system_ability_definition.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
std::shared_ptr<DataShare::DataShareHelper> g_mediaDataShareHelper;

std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(int32_t systemAbilityId)
{
    LOG_INFO("CreateDataShareHelper::CreateFileExtHelper ");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOG_ERROR("CreateDataShareHelper::CreateFileExtHelper Get system ability "
                 "mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    while (remoteObj == nullptr) {
        LOG_ERROR("CreateDataShareHelper::CreateFileExtHelper GetSystemAbility "
                 "Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
}

void MediaDataShareUnitTest::SetUpTestCase(void)
{
    vector<string> perms;
    perms.push_back("ohos.permission.READ_MEDIA");
    perms.push_back("ohos.permission.WRITE_MEDIA");
    perms.push_back("ohos.permission.FILE_ACCESS_MANAGER");
    uint64_t tokenId = 0;
    PermissionUtilsUnitTest::SetAccessTokenPermission("MediaDataShareUnitTest", perms, tokenId);
    ASSERT_TRUE(tokenId != 0);

    LOG_INFO("SetUpTestCase invoked");
    g_mediaDataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_TRUE(g_mediaDataShareHelper != nullptr);
    
    Uri deleteAssetUri(MEDIALIBRARY_DATA_URI);
    DataShare::DataSharePredicates predicates;
    string selections = MEDIA_DATA_DB_ID + " <> 0 ";
    predicates.SetWhereClause(selections);
    int retVal = g_mediaDataShareHelper->Delete(deleteAssetUri, predicates);
    LOG_INFO("SetUpTestCase Delete retVal: %{public}d", retVal);
    EXPECT_EQ((retVal >= 0), true);

    Uri uri(MEDIALIBRARY_DATA_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    double valueD1 = 20.07;
    valuesBucket.Put(MEDIA_DATA_DB_LONGITUDE, valueD1);
    valuesBucket.Put(MEDIA_DATA_DB_TITLE, "dataShareTest003");
    int value1 = 1001;
    valuesBucket.Put(MEDIA_DATA_DB_PARENT_ID, value1);
    retVal = g_mediaDataShareHelper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);

    valuesBucket.Clear();
    double valueD2 = 20.08;
    valuesBucket.Put(MEDIA_DATA_DB_LONGITUDE, valueD2);
    valuesBucket.Put(MEDIA_DATA_DB_TITLE, "dataShareTest004");
    int value2 = 1000;
    valuesBucket.Put(MEDIA_DATA_DB_PARENT_ID, value2);
    retVal = g_mediaDataShareHelper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);

    valuesBucket.Clear();
    double valueD3 = 20.09;
    valuesBucket.Put(MEDIA_DATA_DB_LONGITUDE, valueD3);
    valuesBucket.Put(MEDIA_DATA_DB_TITLE, "dataShareTest005");
    int value3 = 999;
    valuesBucket.Put(MEDIA_DATA_DB_PARENT_ID, value3);
    retVal = g_mediaDataShareHelper->Insert(uri, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    LOG_INFO("SetUpTestCase end");
}

int32_t CreateFile(string displayName)
{
    LOG_INFO("CreateFile::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri createAssetUri(MEDIALIBRARY_DATA_URI + "/" + Media::MEDIA_FILEOPRN + "/" + Media::MEDIA_FILEOPRN_CREATEASSET);
    DataShare::DataShareValuesBucket valuesBucket;
    string relativePath = "Pictures/" + displayName + "/";
    displayName += ".jpg";
    MediaType mediaType = MEDIA_TYPE_IMAGE;
    valuesBucket.Put(MEDIA_DATA_DB_MEDIA_TYPE, mediaType);
    valuesBucket.Put(MEDIA_DATA_DB_NAME, displayName);
    valuesBucket.Put(MEDIA_DATA_DB_RELATIVE_PATH, relativePath);
    int32_t retVal = helper->Insert(createAssetUri, valuesBucket);
    LOG_INFO("CreateFile::File: %{public}s, retVal: %{public}d", (relativePath + displayName).c_str(), retVal);
    EXPECT_EQ((retVal > 0), true);
    if (retVal <= 0) {
        retVal = E_FAIL;
    }
    LOG_INFO("CreateFile::retVal = %{public}d. End", retVal);
    return retVal;
}

int32_t CreateAlbum(string displayName)
{
    LOG_INFO("CreateAlbum::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri createAlbumUri(MEDIALIBRARY_DATA_URI + "/" + MEDIA_ALBUMOPRN + "/" + MEDIA_ALBUMOPRN_CREATEALBUM);
    DataShare::DataShareValuesBucket valuesBucket;
    string dirPath = ROOT_MEDIA_DIR + "Pictures/" + displayName;
    valuesBucket.Put(MEDIA_DATA_DB_FILE_PATH, dirPath);
    valuesBucket.Put(MEDIA_DATA_DB_NAME, displayName);
    auto retVal = helper->Insert(createAlbumUri, valuesBucket);
    LOG_INFO("CreateAlbum::Album: %{public}s, retVal: %{public}d", dirPath.c_str(), retVal);
    EXPECT_EQ((retVal > 0), true);
    if (retVal <= 0) {
        retVal = E_FAIL;
    }
    LOG_INFO("CreateAlbum::retVal = %{public}d. End", retVal);
    return retVal;
}

bool GetFileAsset(unique_ptr<FileAsset> &fileAsset, bool isAlbum, string displayName)
{
    int32_t index = E_FAIL;
    if (isAlbum) {
        index = CreateAlbum(displayName);
    } else {
        index = CreateFile(displayName);
    }
    if (index == E_FAIL) {
        LOG_ERROR("GetFileAsset failed");
        return false;
    }
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    vector<string> columns;
    DataShare::DataSharePredicates predicates;
    string selections = MEDIA_DATA_DB_ID + " = " + to_string(index);
    predicates.SetWhereClause(selections);
    Uri queryFileUri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(queryFileUri, predicates, columns);
    if (resultSet == nullptr) {
        LOG_ERROR("GetFileAsset::resultSet == nullptr");
        return false;
    }

    // Create FetchResult object using the contents of resultSet
    unique_ptr<FetchResult<FileAsset>> fetchFileResult = make_unique<FetchResult<FileAsset>>(move(resultSet));
    if (fetchFileResult->GetCount() <= 0) {
        LOG_ERROR("GetFileAsset::GetCount <= 0");
        return false;
    }

    fileAsset = fetchFileResult->GetFirstObject();
    if (fileAsset == nullptr) {
        LOG_ERROR("GetFileAsset::fileAsset = nullptr.");
        return false;
    }
    return true;
}

void MediaDataShareUnitTest::TearDownTestCase(void)
{
    LOG_INFO("TearDownTestCase invoked");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri deleteAssetUri(MEDIALIBRARY_DATA_URI);
    DataShare::DataSharePredicates predicates;
    string selections = MEDIA_DATA_DB_ID + " <> 0 ";
    predicates.SetWhereClause(selections);
    int retVal = helper->Delete(deleteAssetUri, predicates);
    LOG_INFO("TearDownTestCase Delete retVal: %{public}d", retVal);
    EXPECT_EQ((retVal >= 0), true);

    bool result = helper->Release();
    EXPECT_EQ(result, true);
    LOG_INFO("TearDownTestCase end");
}

void MediaDataShareUnitTest::SetUp(void) {}
void MediaDataShareUnitTest::TearDown(void) {}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(MEDIA_DATA_DB_TITLE, "dataShareTest003");
    predicates.Limit(1, 0);
    vector<string> columns;
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_001, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_002, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_002::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.NotEqualTo(MEDIA_DATA_DB_TITLE, "dataShareTest003");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 2);
    LOG_INFO("MediaDataShare_Predicates_Test_002, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_003, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_003::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.Contains(MEDIA_DATA_DB_TITLE, "dataShareTest");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 3);
    LOG_INFO("MediaDataShare_Predicates_Test_003, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_004, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_004::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.BeginsWith(MEDIA_DATA_DB_TITLE, "dataShare");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 3);
    LOG_INFO("MediaDataShare_Predicates_Test_004, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_005, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_005::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.EndsWith(MEDIA_DATA_DB_TITLE, "003");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_005, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_006, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_006::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.IsNull(MEDIA_DATA_DB_TITLE);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 0);
    LOG_INFO("MediaDataShare_Predicates_Test_006, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_007, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_007::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.IsNotNull(MEDIA_DATA_DB_TITLE);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 3);
    LOG_INFO("MediaDataShare_Predicates_Test_007, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_008, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_008::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.Like(MEDIA_DATA_DB_TITLE, "%Test003");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_008, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_009, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_009::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.Glob(MEDIA_DATA_DB_TITLE, "dataShareTes?003");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_009, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_010, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_010::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.Between(MEDIA_DATA_DB_PARENT_ID, "0", "999");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_010, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_011, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_011::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.NotBetween(MEDIA_DATA_DB_PARENT_ID, "0", "999");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 2);
    LOG_INFO("MediaDataShare_Predicates_Test_011, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_012, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_012::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.GreaterThan(MEDIA_DATA_DB_PARENT_ID, 999);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 2);
    LOG_INFO("MediaDataShare_Predicates_Test_012, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_013, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_013::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.LessThan(MEDIA_DATA_DB_PARENT_ID, 1000);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_013, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_014, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_014::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.GreaterThanOrEqualTo(MEDIA_DATA_DB_PARENT_ID, 1000);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 2);
    LOG_INFO("MediaDataShare_Predicates_Test_014, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_015, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_015::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.LessThanOrEqualTo(MEDIA_DATA_DB_PARENT_ID, 1000);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 2);
    LOG_INFO("MediaDataShare_Predicates_Test_015, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_016, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_016::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(MEDIA_DATA_DB_LONGITUDE, 20.08)
        ->BeginWrap()
        ->EqualTo(MEDIA_DATA_DB_TITLE, "dataShareTest004")
        ->Or()
        ->EqualTo(MEDIA_DATA_DB_PARENT_ID, 1000)
        ->EndWrap();
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_016, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_017, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_017::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(MEDIA_DATA_DB_LONGITUDE, 20.08)->And()->EqualTo(MEDIA_DATA_DB_TITLE, "dataShareTest004");
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int result = 0;
    if (resultSet != nullptr) {
        resultSet->GetRowCount(result);
    }
    EXPECT_EQ(result, 1);
    LOG_INFO("MediaDataShare_Predicates_Test_017, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_018, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_018::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.OrderByAsc(MEDIA_DATA_DB_PARENT_ID);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int columnIndex = 0;
    std::string stringResult = "";
    if (resultSet != nullptr) {
        resultSet->GoToFirstRow();
        resultSet->GetColumnIndex(MEDIA_DATA_DB_TITLE, columnIndex);
        resultSet->GetString(columnIndex, stringResult);
    }
    EXPECT_EQ(stringResult, "dataShareTest005");
    LOG_INFO("MediaDataShare_Predicates_Test_018, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_Predicates_Test_019, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_Predicates_Test_019::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    DataShare::DataSharePredicates predicates;
    predicates.OrderByDesc(MEDIA_DATA_DB_LONGITUDE);
    vector<string> columns;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto resultSet = helper->Query(uri, predicates, columns);
    int columnIndex = 0;
    std::string stringResult = "";
    if (resultSet != nullptr) {
        resultSet->GoToFirstRow();
        resultSet->GetColumnIndex(MEDIA_DATA_DB_TITLE, columnIndex);
        resultSet->GetString(columnIndex, stringResult);
    }
    EXPECT_EQ(stringResult, "dataShareTest005");
    LOG_INFO("MediaDataShare_Predicates_Test_019, End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_ValuesBucket_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_ValuesBucket_Test_001::Start");
    DataShare::DataShareValuesBucket valuesBucket;
    EXPECT_EQ(valuesBucket.IsEmpty(), true);

    valuesBucket.Put(MEDIA_DATA_DB_TITLE, "dataShare_Test_001");
    EXPECT_EQ(valuesBucket.IsEmpty(), false);

    bool isValid;
    DataShare::DataShareValueObject object = valuesBucket.Get(MEDIA_DATA_DB_TITLE, isValid);
    string value = object;
    EXPECT_EQ(value, "dataShare_Test_001");

    valuesBucket.Clear();
    EXPECT_EQ(valuesBucket.IsEmpty(), true);
    LOG_INFO("MediaDataShare_ValuesBucket_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_ValueObject_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_ValueObject_Test_001::Start");
    
    int base = 100;
    DataShare::DataShareValueObject object(base);
    int value = object;
    EXPECT_EQ(value, base);

    int64_t base64 = 100;
    DataShare::DataShareValueObject object64(base64);
    int64_t value64 = object64;
    EXPECT_EQ(value64, base64);

    double baseD = 10.0;
    DataShare::DataShareValueObject objectD(baseD);
    double valueD = objectD;
    EXPECT_EQ(valueD, baseD);

    bool baseB = true;
    DataShare::DataShareValueObject objectB(baseB);
    bool valueB = objectB;
    EXPECT_EQ(valueB, baseB);

    vector<uint8_t> baseV;
    DataShare::DataShareValueObject objectV(baseV);
    vector<uint8_t> valueV = objectV;
    EXPECT_EQ(valueV, baseV);

    DataShare::DataShareValueObject objectCopy(object);
    int valueCopy = objectCopy;
    EXPECT_EQ(valueCopy, value);

    DataShare::DataShareValueObject objectMove(std::move(object));
    int valueMove = objectMove;
    EXPECT_EQ(valueMove, value);

    LOG_INFO("MediaDataShare_ValueObject_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_GetFileTypes_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_GetFileTypes_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    std::vector<std::string> result = helper->GetFileTypes(uri, mimeTypeFilter);
    EXPECT_EQ(result.size(), 0);
    LOG_INFO("MediaDataShare_GetFileTypes_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_OpenRawFile_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_OpenRawFile_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    std::string mode("modetest");
    int result = helper->OpenRawFile(uri, mode);
    EXPECT_EQ(result, 0);
    LOG_INFO("MediaDataShare_OpenRawFile_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_batchInsert_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_batchInsert_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put("name", "ZhangSan");
    valuesBucket.Put("age", 20);
    std::vector<DataShare::DataShareValuesBucket> values;
    values.push_back(valuesBucket);
    int result = helper->BatchInsert(uri, values);
    EXPECT_NE(result, 0);
    LOG_INFO("MediaDataShare_batchInsert_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_NormalizeUri_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_NormalizeUri_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto normalUri = helper->NormalizeUri(uri);
    EXPECT_EQ(normalUri, uri);
    LOG_INFO("MediaDataShare_NormalizeUri_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_DenormalizeUri_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_DenormalizeUri_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    auto denormalUri = helper->DenormalizeUri(uri);
    EXPECT_EQ(denormalUri, uri);
    LOG_INFO("MediaDataShare_DenormalizeUri_Test_001 End");
}

HWTEST_F(MediaDataShareUnitTest, MediaDataShare_GetType_Test_001, TestSize.Level0)
{
    LOG_INFO("MediaDataShare_GetType_Test_001::Start");
    std::shared_ptr<DataShare::DataShareHelper> helper = g_mediaDataShareHelper;
    Uri uri(MEDIALIBRARY_DATA_URI);
    std::string result = helper->GetType(uri);
    EXPECT_NE(result.c_str(), "");
    LOG_INFO("MediaDataShare_GetType_Test_001 End");
}
} // namespace Media
} // namespace OHOS
