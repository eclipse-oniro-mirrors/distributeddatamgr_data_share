/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DATASHARE_ANI_RESULT_SET_H
#define DATASHARE_ANI_RESULT_SET_H

#include "datashare_shared_result_set.h"
#include "ani_observer.h"
#include "ani_subscriber_manager.h"
#include "datashare_helper.h"
namespace OHOS {
using namespace DataShare;

namespace DataShareAni {

struct EnvPtrWrap;
struct VersionWrap;
struct ValuesBucketKvItem;
struct ValueType;
struct PublishedItem;
struct PublishSretParam;
struct GetPublishedDataSretParam;
struct Template;
struct TemplatePredicatesKvItem;
struct ValuesBucketWrap;
struct ChangeInfo;
struct TemplateId;
struct RdbDataChangeNode;
struct PublishedDataChangeNode;

bool GoToFirstRow(int64_t resultSetPtr);
bool GoToLastRow(int64_t resultSetPtr);
bool GoToNextRow(int64_t resultSetPtr);
rust::String GetString(int64_t resultSetPtr, int columnIndex);
int64_t GetLong(int64_t resultSetPtr, int columnIndex);
void Close(int64_t resultSetPtr);
int GetColumnIndex(int64_t resultSetPtr, rust::String columnName);

int64_t DataSharePredicatesNew();
void DataSharePredicatesClean(int64_t predicatesPtr);
void DataSharePredicatesEqualTo(int64_t predicatesPtr, rust::String field, const ValueType& value);
void DataSharePredicatesNotEqualTo(int64_t predicatesPtr, rust::String field, const ValueType& value);
void DataSharePredicatesBeginWrap(int64_t predicatesPtr);
void DataSharePredicatesEndWrap(int64_t predicatesPtr);
void DataSharePredicatesOr(int64_t predicatesPtr);
void DataSharePredicatesAnd(int64_t predicatesPtr);
void DataSharePredicatesContains(int64_t predicatesPtr, rust::String field, rust::String value);
void DataSharePredicatesIsNull(int64_t predicatesPtr, rust::String field);
void DataSharePredicatesIsNotNull(int64_t predicatesPtr, rust::String field);
void DataSharePredicatesLike(int64_t predicatesPtr, rust::String field, rust::String value);
void DataSharePredicatesBetween(int64_t predicatesPtr, rust::String field,
                                const ValueType& low, const ValueType& high);
void DataSharePredicatesGreaterThan(int64_t predicatesPtr, rust::String field, const ValueType& value);
void DataSharePredicatesGreaterThanOrEqualTo(int64_t predicatesPtr, rust::String field, const ValueType& value);
void DataSharePredicatesLessThanOrEqualTo(int64_t predicatesPtr, rust::String field, const ValueType& value);
void DataSharePredicatesLessThan(int64_t predicatesPtr, rust::String field, const ValueType& value);
void DataSharePredicatesOrderByAsc(int64_t predicatesPtr, rust::String field);
void DataSharePredicatesOrderByDesc(int64_t predicatesPtr, rust::String field);
void DataSharePredicatesLimit(int64_t predicatesPtr, double total, double offset);
void DataSharePredicatesGroupBy(int64_t predicatesPtr, rust::Vec<rust::String> field);
void DataSharePredicatesIn(int64_t predicatesPtr, rust::String field, rust::Vec<ValueType> value);
void DataSharePredicatesNotIn(int64_t predicatesPtr, rust::String field, rust::Vec<ValueType> value);

int64_t DataShareNativeCreate(int64_t context, rust::String strUri, bool optionIsUndefined, bool isProxy);

void DataShareNativeClean(int64_t dataShareHelperPtr);

int64_t DataShareNativeQuery(int64_t dataShareHelperPtr, rust::String strUri,
                             int64_t dataSharePredicatesPtr, rust::Vec<rust::String> columns);

int DataShareNativeUpdate(int64_t dataShareHelperPtr, rust::String strUri,
                          int64_t dataSharePredicatesPtr, rust::Vec<ValuesBucketKvItem> bucket);

void DataShareNativePublish(int64_t dataShareHelperPtr, rust::Vec<PublishedItem> data,
                            rust::String bundleName, VersionWrap version, PublishSretParam& sret);

void DataShareNativeGetPublishedData(int64_t dataShareHelperPtr, rust::String bundleName,
                                     GetPublishedDataSretParam& sret);

void DataShareNativeAddTemplate(int64_t dataShareHelperPtr, rust::String uri,
                                rust::String subscriberId, const Template& temp);

void DataShareNativeDelTemplate(int64_t dataShareHelperPtr, rust::String uri, rust::String subscriberId);

int DataShareNativeInsert(int64_t dataShareHelperPtr, rust::String strUri, rust::Vec<ValuesBucketKvItem> bucket);

int DataShareNativeBatchInsert(int64_t dataShareHelperPtr, rust::String strUri, rust::Vec<ValuesBucketWrap> buckets);

int DataShareNativeDelete(int64_t dataShareHelperPtr, rust::String strUri, int64_t dataSharePredicatesPtr);

void DataShareNativeClose(int64_t dataShareHelperPtr);

void DataShareNativeOn(EnvPtrWrap envPtrWrap, rust::String strType, rust::String strUri);

void DataShareNativeOnChangeinfo(EnvPtrWrap envPtrWrap, rust::String event,
                                 int32_t arktype, rust::String strUri);

void DataShareNativeOnRdbDataChange(EnvPtrWrap envPtrWrap, rust::String arktype,
                                    rust::Vec<rust::String> uris, const TemplateId& templateId,
                                    PublishSretParam& sret);

void DataShareNativeOnPublishedDataChange(EnvPtrWrap envPtrWrap, rust::String arktype,
                                          rust::Vec<rust::String> uris, rust::String subscriberId,
                                          PublishSretParam& sret);

void DataShareNativeOff(EnvPtrWrap envPtrWrap, rust::String strType, rust::String strUri);

void DataShareNativeOffChangeinfo(EnvPtrWrap envPtrWrap, rust::String event, int32_t arktype, rust::String strUri);
                                          
void DataShareNativeOffRdbDataChange(EnvPtrWrap envPtrWrap, rust::String arktype,
                                     rust::Vec<rust::String> uris, const TemplateId& templateId,
                                     PublishSretParam& sret);
                                          
void DataShareNativeOffPublishedDataChange(EnvPtrWrap envPtrWrap, rust::String arktype,
                                           rust::Vec<rust::String> uris, rust::String subscriberId,
                                           PublishSretParam& sret);

void DataShareNativeExtensionCallbackInt(double errorCode, rust::string errorMsg, int32_t data, int64_t nativePtr);

void DataShareNativeExtensionCallbackObject(double errorCode, rust::string errorMsg, int64_t ptr, int64_t nativePtr);

void DataShareNativeExtensionCallbackVoid(double errorCode, rust::string errorMsg, int64_t nativePtr);

void ANIRegisterObserver(const std::string &uri, long long dataShareHelperPtr, long long envPtr,
    long long callbackPtr, bool isNotifyDetails = false);

void ANIUnRegisterObserver(const std::string &uri, long long dataShareHelperPtr, long long envPtr,
    bool isNotifyDetails = false);

void ANIUnRegisterObserver(const std::string &uri, long long dataShareHelperPtr, long long envPtr,
    long long callbackPtr, bool isNotifyDetails = false);

static std::map<std::string, std::list<sptr<ANIDataShareObserver>>> observerMap_;

} // namespace DataShareAni
} // namespace OHOS

#endif