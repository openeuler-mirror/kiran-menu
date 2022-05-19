/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#ifdef __cplusplus
extern "C"
{
#endif
    enum PointerEventType
    {
        //鼠标左键按下
        POINTER_EVENT_TYPE_LEFT_BUTTON_PRESS = 1,
        //鼠标中键按下
        POINTER_EVENT_TYPE_MIDDLE_BUTTON_PRESS,
        //鼠标右键按下
        POINTER_EVENT_TYPE_RIGHT_BUTTON_PRESS,
    };
#ifdef __cplusplus
}
#endif
