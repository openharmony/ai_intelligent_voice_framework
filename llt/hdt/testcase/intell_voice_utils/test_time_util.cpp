/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "time_util.h"

namespace OHOS {
namespace IntellVoiceUtils {
TEST_CASE("GetCurrTime", "intell_voice_time") {
    enum TimeFormat format = TIME_FORMAT_DEFAULT;
    std::string str = TimeUtil::GetCurrTime(format);
    REQUIRE(str != "");

    SECTION("continuous time format") {
        TimeFormat format = TIME_FORMAT_CONTINOUS;
        str = TimeUtil::GetCurrTime(format);
        REQUIRE(str != "");
    }
    SECTION("standard time format") {
        TimeFormat format = TIME_FORMAT_STANDARD;
        str = TimeUtil::GetCurrTime(format);
        REQUIRE(str != "");
    }
}
}
}