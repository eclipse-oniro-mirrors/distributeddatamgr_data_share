/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <fuzzer/FuzzedDataProvider.h>

#include "datashareitypesutils_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "datashare_itypes_utils.h"
#include "message_parcel.h"

using namespace OHOS::DataShare;

namespace OHOS {
// Upper bound of consumed random-length strings, keeps a single input cheap.
constexpr size_t MAX_TEXT_LENGTH = 64;
// Max element count of the constructed containers.
constexpr uint8_t MAX_CONTAINER_COUNT = 4;
// Variant selector count of the value types.
constexpr uint8_t VALUE_KIND_COUNT = 4;
// Max count of the predicate operation chain.
constexpr uint8_t MAX_OPERATION_COUNT = 4;
// Pool size of the semantic uri pool.
constexpr size_t URI_POOL_SIZE = 2;

// Semantic uri pool drives the branch splitting of UriUtils-based parsing.
constexpr const char *URI_POOL[] = {
    "datashare://com.example.provider/appendix/table?user=100",
    "datashareproxy://com.example.provider/dataKey",
};

// Dispatch targets of the entry switch, indexed by the first consumed byte.
enum FuzzTarget : uint8_t {
    FUZZ_ROUND_TRIP_PREDICATES = 0,
    FUZZ_ROUND_TRIP_VALUES_BUCKET,
    FUZZ_ROUND_TRIP_TEMPLATE,
    FUZZ_ROUND_TRIP_DATA,
    FUZZ_ROUND_TRIP_UPDATE_OPERATION,
    FUZZ_ROUND_TRIP_PROXY_DATA,
    FUZZ_ROUND_TRIP_CHANGE_NODES,
    FUZZ_ROUND_TRIP_SMALL_OBJECTS,
    FUZZ_RAW_PARCEL_UNMARSHALLING,
};

// Target interface count of this fuzzer, used by the entry switch.
constexpr size_t TARGET_SIZE = 9;

std::string ConsumeUriFromPool(FuzzedDataProvider &fdp)
{
    return URI_POOL[fdp.ConsumeIntegral<uint8_t>() % URI_POOL_SIZE];
}

std::string ConsumeString(FuzzedDataProvider &fdp)
{
    return fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH);
}

DataShareValueObject ConsumeValueObject(FuzzedDataProvider &fdp)
{
    switch (fdp.ConsumeIntegral<uint8_t>() % VALUE_KIND_COUNT) {
        case 0:
            return fdp.ConsumeIntegral<int64_t>();
        case 1:
            return fdp.ConsumeFloatingPoint<double>();
        case 2:
            return ConsumeString(fdp);
        default:
            return fdp.ConsumeBool();
    }
}

// Marshals an object into a parcel and unmarshals it back, covering both
// serialization branches (type/length checks) and deserialization branches.
template<typename T>
void RoundTrip(FuzzedDataProvider &fdp, T &value)
{
    MessageParcel parcel;
    if (!ITypesUtil::Marshalling(value, parcel)) {
        return;
    }
    T result{};
    ITypesUtil::Unmarshalling(result, parcel);
}

void RoundTripPredicatesFuzz(FuzzedDataProvider &fdp)
{
    DataSharePredicates predicates;
    predicates.SetWhereClause(ConsumeString(fdp));
    predicates.SetOrder(ConsumeString(fdp));
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_OPERATION_COUNT;
    for (uint8_t i = 0; i < count; i++) {
        predicates.EqualTo(ConsumeString(fdp), fdp.ConsumeIntegral<int64_t>());
    }
    RoundTrip(fdp, predicates);
}

void RoundTripValuesBucketFuzz(FuzzedDataProvider &fdp)
{
    DataShareValuesBucket bucket;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_CONTAINER_COUNT;
    for (uint8_t i = 0; i < count; i++) {
        std::string key = ConsumeString(fdp);
        bucket.valuesMap[key] = ConsumeValueObject(fdp);
    }
    RoundTrip(fdp, bucket);
}

void RoundTripTemplateFuzz(FuzzedDataProvider &fdp)
{
    Template tplt;
    tplt.update_ = ConsumeString(fdp);
    tplt.scheduler_ = ConsumeString(fdp);
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_CONTAINER_COUNT;
    for (uint8_t i = 0; i < count; i++) {
        std::string key = ConsumeString(fdp);
        tplt.predicates_.emplace_back(key, ConsumeString(fdp));
    }
    RoundTrip(fdp, tplt);
}

void RoundTripDataFuzz(FuzzedDataProvider &fdp)
{
    Data data;
    data.version_ = fdp.ConsumeIntegral<int32_t>();
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_CONTAINER_COUNT;
    for (uint8_t i = 0; i < count; i++) {
        std::string key = ConsumeString(fdp);
        int64_t subscriberId = fdp.ConsumeIntegral<int64_t>();
        data.datas_.emplace_back(key, subscriberId, ConsumeString(fdp));
    }
    RoundTrip(fdp, data);
}

void RoundTripUpdateOperationFuzz(FuzzedDataProvider &fdp)
{
    UpdateOperation operation;
    operation.predicates.SetWhereClause(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
    operation.valuesBucket.valuesMap[fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH)] =
        fdp.ConsumeIntegral<int64_t>();
    RoundTrip(fdp, operation);
    BatchUpdateResult result;
    result.uri = fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH);
    result.codes.push_back(fdp.ConsumeIntegral<int32_t>());
    RoundTrip(fdp, result);
}

void RoundTripProxyDataFuzz(FuzzedDataProvider &fdp)
{
    DataShareProxyData proxyData(ConsumeUriFromPool(fdp), fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
    RoundTrip(fdp, proxyData);
}

void RoundTripChangeNodesFuzz(FuzzedDataProvider &fdp)
{
    RdbChangeNode rdbNode;
    rdbNode.uri_ = ConsumeUriFromPool(fdp);
    rdbNode.templateId_.subscriberId_ = fdp.ConsumeIntegral<int64_t>();
    rdbNode.templateId_.bundleName_ = ConsumeString(fdp);
    RoundTrip(fdp, rdbNode);

    PublishedDataChangeNode publishedNode;
    publishedNode.ownerBundleName_ = ConsumeString(fdp);
    RoundTrip(fdp, publishedNode);
}

void RoundTripSmallObjectsFuzz(FuzzedDataProvider &fdp)
{
    TimedQueryUriInfo timedQueryUriInfo;
    timedQueryUriInfo.uri_ = ConsumeUriFromPool(fdp);
    RoundTrip(fdp, timedQueryUriInfo);

    SubscribeOption subscribeOption;
    subscribeOption.subscribeStatus[ConsumeUriFromPool(fdp)] = fdp.ConsumeBool();
    RoundTrip(fdp, subscribeOption);

    OperationResult operationResult(ConsumeString(fdp), fdp.ConsumeIntegral<int32_t>());
    RoundTrip(fdp, operationResult);
}

// Drives the deserialization branches with raw fuzz bytes, covering the
// type/length mismatch and truncated-buffer fault paths that a round trip
// can never produce.
void RawParcelUnmarshallingFuzz(FuzzedDataProvider &fdp)
{
    std::vector<uint8_t> raw = fdp.ConsumeBytes<uint8_t>(
        fdp.ConsumeIntegral<uint8_t>() % (MAX_TEXT_LENGTH + 1));
    MessageParcel parcel;
    parcel.WriteUInt8Vector(raw);

    DataSharePredicates predicates;
    ITypesUtil::Unmarshalling(predicates, parcel);
    DataShareValuesBucket bucket;
    ITypesUtil::Unmarshalling(bucket, parcel);
    Data data;
    ITypesUtil::Unmarshalling(data, parcel);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    uint8_t target = fdp.ConsumeIntegral<uint8_t>() % OHOS::TARGET_SIZE;
    switch (target) {
        case OHOS::FUZZ_ROUND_TRIP_PREDICATES:
            OHOS::RoundTripPredicatesFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_VALUES_BUCKET:
            OHOS::RoundTripValuesBucketFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_TEMPLATE:
            OHOS::RoundTripTemplateFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_DATA:
            OHOS::RoundTripDataFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_UPDATE_OPERATION:
            OHOS::RoundTripUpdateOperationFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_PROXY_DATA:
            OHOS::RoundTripProxyDataFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_CHANGE_NODES:
            OHOS::RoundTripChangeNodesFuzz(fdp);
            break;
        case OHOS::FUZZ_ROUND_TRIP_SMALL_OBJECTS:
            OHOS::RoundTripSmallObjectsFuzz(fdp);
            break;
        default:
            OHOS::RawParcelUnmarshallingFuzz(fdp);
            break;
    }
    return 0;
}
