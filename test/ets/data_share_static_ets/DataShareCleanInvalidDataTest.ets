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

import { describe, it, expect, TestType, Size, Level, beforeAll, afterAll} from "../../../hypium/index";
import common from "@ohos.app.ability.common";
import { BusinessError } from '@ohos.base';
import dataShare from '@ohos.data.dataShare';

let dseUri = "datashareproxy://com.acts.datasharetest.DataShare/test";

let context: common.UIAbilityContext;
let datashareHelper: dataShare.DataShareHelper | undefined = undefined;
let dsHelper: dataShare.DataShareHelper;
let TAG = "[cleanInvalidDataTestClient]";

export default function dataShareCleanInvalidDataTest() {
  describe('dataShareCleanInvalidDataTest', () => {
    beforeAll(async () => {
      context = AppStorage.get<common.UIAbilityContext>("context") as common.UIAbilityContext;
      try {
        datashareHelper = await dataShare.createDataShareHelper(context, dseUri);
        dsHelper = datashareHelper as dataShare.DataShareHelper;
      } catch(err) {
        let code = (err as BusinessError).code;
        let message = (err as BusinessError).message;
        console.error(`beforeAll error: code: ${code}, message: ${message} `);
        expect(true).assertFail();
      }
    })

    afterAll(async () => {
      try {
        dsHelper = await dataShare.createDataShareHelper(context, dseUri) as dataShare.DataShareHelper;
        await dsHelper.close();
      } catch (err) {
        let code = (err as BusinessError).code;
        let message = (err as BusinessError).message;
        console.error(`afterAll error: code: ${code}, message: ${message} `);
      }
    })

    /**
     * @tc.number    : SUB_DistributedData_DataShare_SDK_CleanInvalidDataStaticTest_0100
     * @tc.name      : dataShareCleanInvalidDataTest001
     * @tc.desc      : add templates with multiple subscriber ids, clean invalid data with partial valid subscriber ids
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('dataShareCleanInvalidDataTest001', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: () => void): Promise<void> => {
      console.log(TAG + "dataShareCleanInvalidDataTest001 start");
      if (datashareHelper != undefined) {
        try {
          let template: dataShare.Template = {
            predicates: {
              "p1": "select name0 as name from TBL00",
              "p2": "select name1 as name from TBL00",
            },
            scheduler: "select remindTimer(strftime('%s', 'now', '+10 seconds'))"
          }
          dsHelper.addTemplate(dseUri, "1", template);
          dsHelper.addTemplate(dseUri, "2", template);
          dsHelper.addTemplate(dseUri, "3", template);
          let validSubscribers: Array<string> = ["1", "2"];
          dsHelper.cleanInvalidData(validSubscribers).then(() => {
            console.info(TAG + "clean success");
            expect(true).assertTrue();
            done();
          }).catch((error) => {
            let err = error as BusinessError;
            console.error(`clean error: code: ${err.code}, message: ${err.message} `);
            expect(true).assertFail();
            done();
          });
        } catch (err) {
          let code = (err as BusinessError).code;
          let message = (err as BusinessError).message;
          console.error(`dataShareCleanInvalidDataTest001 error: code: ${code}, message: ${message} `);
          expect(true).assertFail();
          done();
        }
      } else {
        console.log(TAG + "datashareHelper is undefined");
        expect(true).assertFail();
        done();
      }
      console.log(TAG + "dataShareCleanInvalidDataTest001 end");
    })

    /**
     * @tc.number    : SUB_DistributedData_DataShare_SDK_CleanInvalidDataStaticTest_0200
     * @tc.name      : dataShareCleanInvalidDataTest002
     * @tc.desc      : subscribe rdb data change with the valid template after cleaning
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('dataShareCleanInvalidDataTest002', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: () => void): Promise<void> => {
      console.log(TAG + "dataShareCleanInvalidDataTest002 start");
      if (datashareHelper != undefined) {
        try {
          let template: dataShare.Template = {
            predicates: {
              "p1": "select name0 as name from TBL00",
            },
            scheduler: "select remindTimer(strftime('%s', 'now', '+10 seconds'))"
          }
          dsHelper.addTemplate(dseUri, "1", template);
          dsHelper.addTemplate(dseUri, "2", template);
          await dsHelper.cleanInvalidData(["1", "2"]);
          let templateId: dataShare.TemplateId = {
            subscriberId: "1",
            bundleNameOfOwner: "com.acts.datasharetest.DataShare"
          }
          let result: Array<dataShare.OperationResult> = dsHelper.on("rdbDataChange", [dseUri], templateId,
            (err: BusinessError, node: dataShare.RdbDataChangeNode) => {
              console.info(TAG + "callback receive rdbDataChange");
            });
          console.info(TAG + `on result: ${JSON.stringify(result)}`);
          expect(result.length > 0).assertTrue();
          expect(result[0].result == 0).assertTrue();
          done();
        } catch (err) {
          let code = (err as BusinessError).code;
          let message = (err as BusinessError).message;
          console.error(`dataShareCleanInvalidDataTest002 error: code: ${code}, message: ${message} `);
          expect(true).assertFail();
          done();
        }
      } else {
        console.log(TAG + "datashareHelper is undefined");
        expect(true).assertFail();
        done();
      }
      console.log(TAG + "dataShareCleanInvalidDataTest002 end");
    })

    /**
     * @tc.number    : SUB_DistributedData_DataShare_SDK_CleanInvalidDataStaticTest_0300
     * @tc.name      : dataShareCleanInvalidDataTest003
     * @tc.desc      : clean invalid data after close, expect 15700013
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('dataShareCleanInvalidDataTest003', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: () => void): Promise<void> => {
      console.log(TAG + "dataShareCleanInvalidDataTest003 start");
      if (datashareHelper != undefined) {
        try {
          await dsHelper.close();
          dsHelper.cleanInvalidData(["1"]).then(() => {
            console.error(`clean should not success`);
            expect(true).assertFail();
            done();
          }).catch((error) => {
            let err = error as BusinessError;
            console.error(`clean error: code: ${err.code}, message: ${err.message} `);
            expect(err.code == 15700013).assertTrue();
            done();
          });
        } catch (err) {
          let code = (err as BusinessError).code;
          let message = (err as BusinessError).message;
          console.error(`dataShareCleanInvalidDataTest003 error: code: ${code}, message: ${message} `);
          expect(true).assertFail();
          done();
        }
      } else {
        console.log(TAG + "datashareHelper is undefined");
        expect(true).assertFail();
        done();
      }
      console.log(TAG + "dataShareCleanInvalidDataTest003 end");
    })

    /**
     * @tc.number    : SUB_DistributedData_DataShare_SDK_CleanInvalidDataStaticTest_0400
     * @tc.name      : dataShareCleanInvalidDataTest004
     * @tc.desc      : clean invalid data with invalid parameters, expect 401
     * @tc.size      : MediumTest
     * @tc.type      : Function
     * @tc.level     : Level 1
     */
    it('dataShareCleanInvalidDataTest004', TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1, async (done: () => void): Promise<void> => {
      console.log(TAG + "dataShareCleanInvalidDataTest004 start");
      if (datashareHelper != undefined) {
        try {
          let emptyFormIds: Array<string> = [];
          dsHelper.cleanInvalidData(emptyFormIds).then(() => {
            console.error(`empty array should not success`);
            expect(true).assertFail();
            done();
          }).catch((error) => {
            let err = error as BusinessError;
            console.error(`empty error: code: ${err.code}, message: ${err.message} `);
            expect(err.code == 401).assertTrue();
            let overSizeFormIds: Array<string> = [];
            for (let i = 0; i < 51; i++) {
              overSizeFormIds.push(`${i}`);
            }
            dsHelper.cleanInvalidData(overSizeFormIds).then(() => {
              console.error(`over size should not success`);
              expect(true).assertFail();
              done();
            }).catch((error2) => {
              let err2 = error2 as BusinessError;
              console.error(`over size error: code: ${err2.code}, message: ${err2.message} `);
              expect(err2.code == 401).assertTrue();
              done();
            });
          });
        } catch (err) {
          let code = (err as BusinessError).code;
          let message = (err as BusinessError).message;
          console.error(`dataShareCleanInvalidDataTest004 error: code: ${code}, message: ${message} `);
          expect(true).assertFail();
          done();
        }
      } else {
        console.log(TAG + "datashareHelper is undefined");
        expect(true).assertFail();
        done();
      }
      console.log(TAG + "dataShareCleanInvalidDataTest004 end");
    })
  })
}
