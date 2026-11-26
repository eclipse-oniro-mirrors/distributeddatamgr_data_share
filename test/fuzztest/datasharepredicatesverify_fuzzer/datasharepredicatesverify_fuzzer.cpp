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

#include "datasharepredicatesverify_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "datashare_predicates.h"
#include "datashare_predicates_verify.h"

using namespace OHOS::DataShare;

namespace OHOS {
// Upper bound of consumed random-length strings, keeps a single input cheap.
constexpr size_t MAX_TEXT_LENGTH = 64;
// Max count of the randomly composed predicate operation chain.
constexpr uint8_t MAX_OPERATION_COUNT = 6;
// Selector count of the predicate operation kinds.
constexpr uint8_t OPERATION_KIND_COUNT = 10;

// Dispatch targets of the entry switch, indexed by the first consumed byte.
enum FuzzTarget : uint8_t {
    FUZZ_BUILD_AND_VERIFY = 0,
    FUZZ_QUERY_LANGUAGE_GETTERS,
    FUZZ_SERIALIZE_ROUND_TRIP,
};

// Target interface count of this fuzzer, used by the entry switch.
constexpr size_t TARGET_SIZE = 3;

std::string ConsumeString(FuzzedDataProvider &fdp)
{
    return fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH);
}

// Composes a random predicate operation chain from the operation pool, then
// drives every verify branch of DataSharePredicatesVerify.
void BuildAndVerifyFuzz(FuzzedDataProvider &fdp)
{
    DataSharePredicates predicates;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_OPERATION_COUNT;
    for (uint8_t i = 0; i < count; i++) {
        std::string field = ConsumeString(fdp);
        std::string value = ConsumeString(fdp);
        switch (fdp.ConsumeIntegral<uint8_t>() % OPERATION_KIND_COUNT) {
            case 0:
                predicates.EqualTo(field, value);
                break;
            case 1:
                predicates.NotEqualTo(field, value);
                break;
            case 2:
                predicates.GreaterThan(field, value);
                break;
            case 3:
                predicates.LessThan(field, value);
                break;
            case 4:
                predicates.Contains(field, value);
                break;
            case 5:
                predicates.BeginsWith(field, value);
                break;
            case 6:
                predicates.Like(field, value);
                break;
            case 7:
                predicates.Glob(field, value);
                break;
            case 8:
                predicates.IsNull(field);
                break;
            default:
                predicates.OrderByAsc(field);
                break;
        }
    }
    predicates.LimitBy(fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeIntegral<int32_t>());
    predicates.Offset(fdp.ConsumeIntegral<int32_t>());

    DataSharePredicatesVerify verifier;
    verifier.VerifyPredicates(predicates);
}

// Non-empty whereClause/order switches settingMode_ to QUERY_LANGUAGE and
// drives the getters of the predicates.
void QueryLanguageGettersFuzz(FuzzedDataProvider &fdp)
{
    DataSharePredicates predicates;
    predicates.SetWhereClause(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
    predicates.SetOrder(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));

    predicates.GetWhereClause();
    predicates.GetOrder();
    predicates.GetSettingMode();
    predicates.GetOperationList();
}

// SetOperationList with fuzz-driven operations also switches the setting
// mode to QUERY_LANGUAGE, driving the serialization of the operation list.
void SerializeRoundTripFuzz(FuzzedDataProvider &fdp)
{
    std::vector<OperationItem> operations;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % MAX_OPERATION_COUNT;
    for (uint8_t i = 0; i < count; i++) {
        OperationItem item;
        item.operation = fdp.ConsumeIntegral<int32_t>();
        item.singleParams.push_back(fdp.ConsumeIntegral<int64_t>());
        item.para1 = fdp.ConsumeIntegral<double>();
        operations.push_back(item);
    }
    DataSharePredicates predicates;
    predicates.SetOperationList(operations);

    DataSharePredicatesVerify verifier;
    verifier.VerifyPredicates(predicates);
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
        case OHOS::FUZZ_BUILD_AND_VERIFY:
            OHOS::BuildAndVerifyFuzz(fdp);
            break;
        case OHOS::FUZZ_QUERY_LANGUAGE_GETTERS:
            OHOS::QueryLanguageGettersFuzz(fdp);
            break;
        default:
            OHOS::SerializeRoundTripFuzz(fdp);
            break;
    }
    return 0;
}
