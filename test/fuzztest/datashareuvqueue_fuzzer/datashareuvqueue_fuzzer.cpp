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

#include "datashareuvqueue_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "datashare_uv_queue.h"

using namespace OHOS::DataShare;

namespace OHOS {
// Dispatch targets of the entry switch, indexed by the first consumed byte.
enum FuzzTarget : uint8_t {
    FUZZ_SYNC_CALL = 0,
    FUZZ_CHECK_FUNC,
};

// Target interface count of this fuzzer, used by the entry switch.
constexpr size_t TARGET_SIZE = 2;

// A default-constructed queue has no napi env or uv loop, which drives the
// null-guard branches of JsSyncCall and StsSyncCall (safe fallback paths of
// the provider-side queue when the JS runtime is absent).
void SyncCallFuzz(FuzzedDataProvider &fdp)
{
    DataShareUvQueue queue;
    if (fdp.ConsumeBool()) {
        queue.JsSyncCall();
    } else {
        queue.StsSyncCall();
    }
}

// CheckFuncAndExec with and without a callback covers both the direct-exec
// and no-op branches.
void CheckFuncFuzz(FuzzedDataProvider &fdp)
{
    DataShareUvQueue queue;
    if (fdp.ConsumeBool()) {
        queue.CheckFuncAndExec([]() { return true; });
    } else {
        queue.CheckFuncAndExec(DataShareUvQueue::BoolFunc());
    }
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
        case OHOS::FUZZ_SYNC_CALL:
            OHOS::SyncCallFuzz(fdp);
            break;
        default:
            OHOS::CheckFuncFuzz(fdp);
            break;
    }
    return 0;
}
