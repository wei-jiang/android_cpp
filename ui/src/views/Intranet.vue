<template>
  <div class="intranet">
    <div v-show="wifi_ip">
      <h3>{{address}}</h3>
      <!-- can not use v-if, cause not be able to get element -->
      <canvas id="addr_qr"></canvas>
      <p>
        请连入
        <b>同一内网</b>，用浏览器上传/查看文件
      </p>
      <p class="prompt">提示：手机接上充电器后可当服务器用；否则过一段时间会自动休眠，导致文件服务器不可访问。</p>
      <div v-if="isAboveAndroid6">
        <div>或把应用<i>加入</i>电源管理白名单</div>
        <button @click="req_whitelist">加入白名单</button>
      </div>
    </div>
    <div v-show="!wifi_ip">
      <h3>wifi未连接，请连入内网后重试！</h3>
    </div>
  </div>
</template>

<script>
import QRious from "qrious";
import cfg from "../common/config";
import util from "../common/util";
export default {
  name: "Intranet",
  props: {
    msg: String
  },
  created: async function() {
    cpp.isAndroidVerGt(22, (flag)=>{
      this.isAboveAndroid6 = flag;
      console.log(`this.isAboveAndroid6=${this.isAboveAndroid6}`)
    });
  },
  destroyed() {},
  mounted() {
    this.gen_qr_address();
  },
  data() {
    return {
      wifi_ip: ""
    };
  },
  computed: {
    address() {
      return `http://${this.wifi_ip}:${cfg.svr_port}`;
    }
  },
  methods: {
    req_whitelist() {
      cpp.isIgnoringBatteryOptimizations(
        (responce)=> {
          console.log("isIgnoringBatteryOptimizations: " + responce);
          if (responce == "false") {
            cpp.requestOptimizations(
              responce => {
                console.log(responce);
              },
              error => {
                console.error("BatteryOptimizations Request Error" + error);
              }
            );
          } else {
            console.log("Application already Ignoring Battery Optimizations, show menu");
            cpp.requestOptimizationsMenu(
              ()=>{

              },
              ()=>{}
            )
          }
        },
        error => {
          console.error("IsIgnoringBatteryOptimizations Error" + error);
        }
      );
    },
    gen_qr_address() {
      networkinterface.getWiFiIPAddress(
        info => {
          this.wifi_ip = info.ip;
          const qr = new QRious({
            size: 200,
            element: document.getElementById("addr_qr"),
            value: this.address
          });
        },
        err => {}
      );
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
canvas {
  margin: 0.3em auto;
  width: 200px;
  height: 200px;
}
.prompt {
  margin: 2em;
  text-align: left;
}
.intranet {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}
button {
  border-radius: 1em;
  font-size: 1.2em;
  padding: 0 1em;
  margin-top: 0.5em;
}
b {
  color: red;
}
i{
  color: green;
}
</style>
