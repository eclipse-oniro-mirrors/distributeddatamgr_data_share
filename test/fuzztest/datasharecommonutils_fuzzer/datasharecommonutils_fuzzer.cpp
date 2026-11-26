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

#include "datasharecommonutils_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "datashare_string_utils.h"
#include "datashare_uri_utils.h"

using namespace OHOS::DataShare;

namespace OHOS {
// Upper bound of consumed random-length strings, keeps a single input cheap.
constexpr size_t MAX_TEXT_LENGTH = 64;
// Pool size of the semantic uri pool.
constexpr size_t URI_POOL_SIZE = 5;

// Semantic uri pool drives the branch splitting of DataShareUriUtils
// (user param, SAID extraction, first path segment and query splitting).
constexpr const char *URI_POOL[] = {
    "datashare://com.example.provider/appendix/table?user=100",
    "datashareproxy://com.example.provider/dataKey",
    "datashare:///appendix/table/SAID=1001",
    "datashare://com.ohos.settingsdata/settings/settings/settings",
    "",
};

// Dispatch targets of the entry switch, indexed by the first consumed byte.
enum FuzzTarget : uint8_t {
    FUZZ_URI_ANALYZE = 0,
    FUZZ_URI_QUERY_PARAMS,
    FUZZ_URI_USER_AND_SAID,
    FUZZ_URI_FORMAT,
    FUZZ_STRING_ANONYMOUS,
    FUZZ_STRING_REMOVE_QUERY,
    FUZZ_RANDOM_NUMBER,
};

// Target interface count of this fuzzer, used by the entry switch.
constexpr size_t TARGET_SIZE = 7;

// Drives ExtractFirstPathSegment and Strtoul branches (decimal parse,
// overflow and invalid-format fault paths).
void UriAnalyzeFuzz(FuzzedDataProvider &fdp)
{
    DataShareUriUtils::ExtractFirstPathSegment(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
    DataShareUriUtils::Strtoul(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
}

void UriQueryParamsFuzz(FuzzedDataProvider &fdp)
{
    DataShareUriUtils::GetQueryParams(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
}

void UriUserAndSaidFuzz(FuzzedDataProvider &fdp)
{
    std::string uri = fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH);
    DataShareUriUtils::GetUserFromUri(uri);
    DataShareUriUtils::GetSystemAbilityId(uri);
}

void UriFormatFuzz(FuzzedDataProvider &fdp)
{
    DataShareUriUtils::FormatUri(URI_POOL[fdp.ConsumeIntegral<uint8_t>() % URI_POOL_SIZE]);
}

// Anonymous/Change drive the masking branches of DataShareStringUtils.
void StringAnonymousFuzz(FuzzedDataProvider &fdp)
{
    DataShareStringUtils::Anonymous(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
    DataShareStringUtils::Change(fdp.ConsumeRandomLengthString(MAX_TEXT_LENGTH));
}

// RemoveFromQuery splits the query component off the uri variants.
void StringRemoveQueryFuzz(FuzzedDataProvider &fdp)
{
    std::string uri = URI_POOL[fdp.ConsumeIntegral<uint8_t>() % URI_POOL_SIZE];
    DataShareStringUtils::RemoveFromQuery(uri);
}

void RandomNumberFuzz(FuzzedDataProvider &fdp)
{
    int32_t min = fdp.ConsumeIntegral<int32_t>();
    int32_t max = fdp.ConsumeIntegral<int32_t>();
    DataShareStringUtils::GetRandomNumber(min, max);
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
        case OHOS::FUZZ_URI_ANALYZE:
            OHOS::UriAnalyzeFuzz(fdp);
            break;
        case OHOS::FUZZ_URI_QUERY_PARAMS:
            OHOS::UriQueryParamsFuzz(fdp);
            break;
        case OHOS::FUZZ_URI_USER_AND_SAID:
            OHOS::UriUserAndSaidFuzz(fdp);
            break;
        case OHOS::FUZZ_URI_FORMAT:
            OHOS::UriFormatFuzz(fdp);
            break;
        case OHOS::FUZZ_STRING_ANONYMOUS:
            OHOS::StringAnonymousFuzz(fdp);
            break;
        case OHOS::FUZZ_STRING_REMOVE_QUERY:
            OHOS::StringRemoveQueryFuzz(fdp);
            break;
        default:
            OHOS::RandomNumberFuzz(fdp);
            break;
    }
    return 0;
}
