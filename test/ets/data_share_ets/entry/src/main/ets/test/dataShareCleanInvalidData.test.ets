/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect, Level, Size, TestType } from "@ohos/hypium";
import common from '@ohos.app.ability.common';
import { AppStorage } from '@ohos.arkui.stateManagement';
import { BusinessError } from "@ohos.base";
import dataShare from '@ohos.data.dataShare';

let TAG = "[dataShareCleanInvalidDataTest]";
let context: common.UIAbilityContext;
let cardUri = "datashareproxy://com.example.myapplication/entry/DB00/TBL00";
let bundleNameOfOwner = "com.example.myapplication";

export default function DataShareCleanInvalidDataTest() {
  describe('DataShareCleanInvalidDataTest', () => {
    beforeAll(async () => {
      context = AppStorage.get<common.UIAbilityContext>('TestAbilityContext') as common.UIAbilityContext;
      console.info(TAG + 'beforeAll');
    })
    beforeEach(async () => {
      console.info(TAG + 'beforeEach');
    })
    afterEach(async () => {
      console.info(TAG + 'afterEach');
    })
    afterAll(async () => {
      console.info(TAG + 'afterAll');
    })

    /**
    * @tc.number: SUB_DistributedData_DataShare_SDK_CleanInvalidDataTest_0100
    * @tc.name: cleanInvalidDataTest001
    * @tc.desc: add templates with multiple subscriber ids, clean invalid data with partial valid subscriber ids
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it('cleanInvalidDataTest001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      console.info(TAG + "************* cleanInvalidDataTest001 start *************");
      try {
        let helper = await dataShare.createDataShareHelper(context, cardUri);
        let template: dataShare.Template = {
          predicates: {
            "p1": "select name0 as name from TBL00",
            "p2": "select name1 as name from TBL00",
          },
          scheduler: "select remindTimer(strftime('%s', 'now', '+10 seconds'))"
        }
        helper.addTemplate(cardUri, "1", template);
        helper.addTemplate(cardUri, "2", template);
        helper.addTemplate(cardUri, "3", template);
        let validSubscribers: Array<string> = ["1", "2"];
        helper.cleanInvalidData(validSubscribers).then(() => {
          console.info(TAG + "cleanInvalidDataTest001 clean success");
          expect(true).assertTrue();
          done();
        }).catch((error) => {
          let err = error as BusinessError;
          console.error(TAG + `cleanInvalidDataTest001 error: code: ${err.code}, message: ${err.message} `);
          expect(true).assertFail();
          done();
        });
      } catch (err) {
        let e = err as BusinessError;
        console.error(TAG + `cleanInvalidDataTest001 catch error: code: ${e.code}, message: ${e.message} `);
        expect(true).assertFail();
        done();
      }
      console.info(TAG + "************* cleanInvalidDataTest001 end *************");
    })

    /**
    * @tc.number: SUB_DistributedData_DataShare_SDK_CleanInvalidDataTest_0200
    * @tc.name: cleanInvalidDataTest002
    * @tc.desc: subscribe rdb data change with the valid template after cleaning
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it('cleanInvalidDataTest002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      console.info(TAG + "************* cleanInvalidDataTest002 start *************");
      try {
        let helper = await dataShare.createDataShareHelper(context, cardUri);
        let template: dataShare.Template = {
          predicates: {
            "p1": "select name0 as name from TBL00",
          },
          scheduler: "select remindTimer(strftime('%s', 'now', '+10 seconds'))"
        }
        helper.addTemplate(cardUri, "1", template);
        helper.addTemplate(cardUri, "2", template);
        await helper.cleanInvalidData(["1", "2"]);
        let templateId: dataShare.TemplateId = {
          subscriberId: "1",
          bundleNameOfOwner: bundleNameOfOwner
        }
        let result: Array<dataShare.OperationResult> =
          helper.on("rdbDataChange", [cardUri], templateId, (err: BusinessError, node: dataShare.RdbDataChangeNode) => {
            console.info(TAG + "cleanInvalidDataTest002 callback receive rdbDataChange");
          });
        console.info(TAG + `cleanInvalidDataTest002 on result: ${JSON.stringify(result)}`);
        expect(result.length > 0).assertTrue();
        expect(result[0].result == 0).assertTrue();
        done();
      } catch (err) {
        let e = err as BusinessError;
        console.error(TAG + `cleanInvalidDataTest002 error: code: ${e.code}, message: ${e.message} `);
        expect(true).assertFail();
        done();
      }
      console.info(TAG + "************* cleanInvalidDataTest002 end *************");
    })

    /**
    * @tc.number: SUB_DistributedData_DataShare_SDK_CleanInvalidDataTest_0300
    * @tc.name: cleanInvalidDataTest003
    * @tc.desc: clean invalid data after close, expect 15700013
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it('cleanInvalidDataTest003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      console.info(TAG + "************* cleanInvalidDataTest003 start *************");
      try {
        let helper = await dataShare.createDataShareHelper(context, cardUri);
        await helper.close();
        helper.cleanInvalidData(["1"]).then(() => {
          console.error(TAG + "cleanInvalidDataTest003 clean should not success");
          expect(true).assertFail();
          done();
        }).catch((error) => {
          let err = error as BusinessError;
          console.info(TAG + `cleanInvalidDataTest003 error: code: ${err.code}, message: ${err.message} `);
          expect(err.code == 15700013).assertTrue();
          done();
        });
      } catch (err) {
        let e = err as BusinessError;
        console.error(TAG + `cleanInvalidDataTest003 catch error: code: ${e.code}, message: ${e.message} `);
        expect(true).assertFail();
        done();
      }
      console.info(TAG + "************* cleanInvalidDataTest003 end *************");
    })

    /**
    * @tc.number: SUB_DistributedData_DataShare_SDK_CleanInvalidDataTest_0400
    * @tc.name: cleanInvalidDataTest004
    * @tc.desc: clean invalid data with invalid parameters, expect 401
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it('cleanInvalidDataTest004', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: Function) => {
      console.info(TAG + "************* cleanInvalidDataTest004 start *************");
      try {
        let helper = await dataShare.createDataShareHelper(context, cardUri);
        let emptyFormIds: Array<string> = [];
        helper.cleanInvalidData(emptyFormIds).then(() => {
          console.error(TAG + "cleanInvalidDataTest004 empty array should not success");
          expect(true).assertFail();
          done();
        }).catch((error) => {
          let err = error as BusinessError;
          console.info(TAG + `cleanInvalidDataTest004 empty error: code: ${err.code}, message: ${err.message} `);
          expect(err.code == 401).assertTrue();
          let overSizeFormIds: Array<string> = [];
          for (let i = 0; i < 51; i++) {
            overSizeFormIds.push(`${i}`);
          }
          helper.cleanInvalidData(overSizeFormIds).then(() => {
            console.error(TAG + "cleanInvalidDataTest004 over size should not success");
            expect(true).assertFail();
            done();
          }).catch((error2) => {
            let err2 = error2 as BusinessError;
            console.info(TAG + `cleanInvalidDataTest004 over size error: code: ${err2.code}, message: ${err2.message} `);
            expect(err2.code == 401).assertTrue();
            done();
          });
        });
      } catch (err) {
        let e = err as BusinessError;
        console.error(TAG + `cleanInvalidDataTest004 catch error: code: ${e.code}, message: ${e.message} `);
        expect(true).assertFail();
        done();
      }
      console.info(TAG + "************* cleanInvalidDataTest004 end *************");
    })
  })
}
