<template>
  <div class="intranet">
    <div v-show="wifi_ip">
      <h3>http://{{wifi_ip}}:<span class="chg-port" @click.prevent="chg_http_port">{{port}}</span></h3>
      <!-- can not use v-if, cause not be able to get element -->
      <canvas id="addr_qr"></canvas>
      <p>
        {{$t('please-cnn')}}<b>{{$t('same-innet')}}</b>{{$t('browse')}}
      </p>
      <p>{{$t('upload-prompt')}}</p>
      <div v-if="isAboveAndroid6">
        <div>{{$t('upload-try')}}</div>
        <button @click="req_whitelist">{{$t('whitelist')}}</button>
      </div>
    </div>
    <div v-show="!wifi_ip">
      <h3>{{$t('wifi-disconnected')}}</h3>
    </div>
  </div>
</template>

<script>
import QRious from "qrious";
import util from "@/common/util";
import WS from "@/ws";
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
    this.port = util.http_port();
  },
  data() {
    return {
      wifi_ip: "",
      port: 57000
    };
  },
  computed: {
    address() {
      return `http://${this.wifi_ip}:${this.port}`;
    }
  },
  methods: {
    chg_http_port(){
      let new_port = prompt(this.$t('new-port'), util.http_port());
      if (new_port === null) {
        return; //break out of the function early
      }
      new_port = parseInt(new_port)
      if( isNaN(new_port) ) return util.show_alert_top_tm( this.$t('invalid-format') )
      if( new_port <= 1024 || new_port > 65534) return util.show_alert_top_tm( this.$t('invalid-ports') )
      db.svr.findAndUpdate({}, s => {
        s.http_port = new_port;
      });
      cpp.restart(new_port);
      console.log('new http-port: ' + util.http_port() )
      this.port = util.http_port();
      window.ws = new WS();
      ws.init();
    },
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
.chg-port{
  background-color: aquamarine;
}
p {
  margin: 0.7em 1.7em;
  text-align: left;
}
.svr_addr{
  display: flex;
  justify-content: space-between;
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
