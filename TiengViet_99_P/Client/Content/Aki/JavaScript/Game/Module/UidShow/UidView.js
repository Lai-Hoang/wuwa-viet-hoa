"use strict";
Object.defineProperty(exports, "__esModule", { value: !0 }), (exports.UidView = void 0);
const UE = require("ue"),
  EventDefine_1 = require("../../Common/Event/EventDefine"),
  EventSystem_1 = require("../../Common/Event/EventSystem"),
  ModelManager_1 = require("../../Manager/ModelManager"),
  UiViewBase_1 = require("../../Ui/Base/UiViewBase");
  const UiManager_1 = require("../../Ui/UiManager")
class UidView extends UiViewBase_1.UiViewBase {
  constructor() {
    super(...arguments),
      (this.k4_ = (e) => {
        var t = this.GetText(0);
        t && t.SetUIActive(e);
      });
  }
  OnRegisterComponent() {
    this.ComponentRegisterInfos = [[0, UE.UIText]];
  }
  OnAddEventListener() {
    EventSystem_1.EventSystem.Add(EventDefine_1.EEventName.OnPreparePhotoScreenShot, this.k4_);
  }
  OnRemoveEventListener() {
    EventSystem_1.EventSystem.Remove(EventDefine_1.EEventName.OnPreparePhotoScreenShot, this.k4_);
  }
  SetUid(uidString) {
    const UiText = this.GetText(0);
    UiText.SetText(uidString);
    ModelManager_1.ModelManager.FunctionModel.PlayerId = uidString;
  }
  OnStart() {
    // thank dicord:hxite share this.
    // cảm ơn dicord:hxite đã share cái đổi màu LGBT này :).
    // UiManager_1.UiManager.CloseView("BattleView"); // cai này tắt ui
    // UiManager_1.UiManager.CloseView("UidView"); //tat uid ngoai man hinh chinh
    const uid = "Chào mọi người!";
    this.SetUid("có gì đâu mà nhìn");
    setTimeout(() => {
      // const UiManager_1 = require("../Ui/UiManager");
      const UE = require("ue");
      // const ControllerManagerBase_1 = require("../../Core/Framework/ControllerManagerBase");

      const UiText = UiManager_1.UiManager.GetViewByName("UidView")?.GetText(0);

      if (!UiText) {
        console.error("UidView or Text element 0 not found. Cannot set text or color.");
        return;
      }
      
      UiText.SetText(uid);

      let hue = 0;
      const saturation = 1;
      const lightness = 0.5;
      const animationSpeed = 1;
      const updateInterval = 50; // ms

      function hslToRgb(h, s, l) {
        let c = (1 - Math.abs(2 * l - 1)) * s;
        let x = c * (1 - Math.abs((h / 60) % 2 - 1));
        let m = l - c / 2;
        let r = 0, g = 0, b = 0;

        if (0 <= h && h < 60) {
          r = c; g = x; b = 0;
        } else if (60 <= h && h < 120) {
          r = x; g = c; b = 0;
        } else if (120 <= h && h < 180) {
          r = 0; g = c; b = x;
        } else if (180 <= h && h < 240) {
          r = 0; g = x; b = c;
        } else if (240 <= h && h < 300) {
          r = x; g = 0; b = c;
        } else if (300 <= h && h < 360) {
          r = c; g = 0; b = x;
        }
        r = Math.round((r + m) * 255);
        g = Math.round((g + m) * 255);
        b = Math.round((b + m) * 255);
        return { r, g, b };
      }

      function componentToHex(c) {
        const hex = c.toString(16);
        return hex.length == 1 ? "0" + hex : hex;
      }

      function rgbToHex(r, g, b) {
        return componentToHex(r) + componentToHex(g) + componentToHex(b);
      }

      const colorIntervalId = setInterval(() => {
        if (!UiText?.IsValid()) {
          console.warn("UiText is no longer valid. Stopping color animation.");
          clearInterval(colorIntervalId);
          return;
        }
        hue = (hue + animationSpeed) % 360;
        
        const rgbColor = hslToRgb(hue, saturation, lightness);
        const hexColorString = rgbToHex(rgbColor.r, rgbColor.g, rgbColor.b);

        UiText.SetColor(UE.Color.FromHex(hexColorString));

      }, updateInterval);

    }, 100);
  }
}
exports.UidView = UidView;
//# sourceMappingURL=UidView.js.map
