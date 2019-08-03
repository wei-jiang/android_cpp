<template>
  <div class="intranet">
    <div v-show="wifi_ip">
      <h3>
        http://{{wifi_ip}}:
        <span class="chg-port" @click.prevent="chg_http_port">{{port}}</span>
        <span @click.prevent="chg_http_port">&#9756;</span>
      </h3>
      <!-- can not use v-if, cause not be able to get element -->
      <canvas id="addr_qr"></canvas>
      <p>
        {{$t('please-cnn')}}
        <b>{{$t('same-innet')}}</b>
        {{$t('browse')}}
      </p>
      <h3 class="socks">
        socks5: {{wifi_ip}}:
        <span class="chg-port" @click.prevent="chg_socks_port">{{socks_port}}</span>
        <span @click.prevent="chg_socks_port">&#9756;</span>
      </h3>
      {{address}}/proxy.pac
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
    cpp.isAndroidVerGt(22, flag => {
      this.isAboveAndroid6 = flag;
      console.log(`this.isAboveAndroid6=${this.isAboveAndroid6}`);
    });
    this.$root.$on("chg_http_port_back", this.chg_http_port_back);
    this.$root.$on("chg_socks_port_back", this.chg_socks_port_back);
  },
  destroyed() {
    this.$root.$off("chg_http_port_back", this.chg_http_port_back);
    this.$root.$off("chg_socks_port_back", this.chg_socks_port_back);
  },
  mounted() {
    this.gen_qr_address();
    this.port = util.http_port();
    this.socks_port = util.socks_port();
  },
  data() {
    return {
      wifi_ip: "",
      port: 57000,
      socks_port: 57100
    };
  },
  computed: {
    address() {
      return `http://${this.wifi_ip}:${this.port}`;
    }
  },
  methods: {
    chg_socks_port_back(data) {
      if (data.ret == 0) {
        db.svr.findAndUpdate({}, s => {
          s.socks_port = data.port;
        });
        util.write_socks_pac(this.wifi_ip, util.socks_port());
        this.socks_port = util.socks_port();
        util.show_alert_top_tm(this.$t("chg-port-success"));
      } else {
        util.show_error_top(this.$t("chg-port-failed"));
      }
    },
    chg_http_port_back(data) {
      if (data.ret == 0) {
        db.svr.findAndUpdate({}, s => {
          s.http_port = data.port;
        });
        console.log("new http-port: " + util.http_port());
        this.port = util.http_port();
        window.ws = new WS();
        ws.init();
        this.refresh_qr();
        util.show_alert_top_tm(this.$t("chg-port-success"));
      } else {
        util.show_error_top(this.$t("chg-port-failed"));
      }
    },
    chg_socks_port() {
      let new_port = prompt(this.$t("new-port"), util.socks_port());
      if (new_port === null) {
        return; //break out of the function early
      }
      new_port = parseInt(new_port);
      if (isNaN(new_port))
        return util.show_alert_top_tm(this.$t("invalid-format"));
      if (new_port <= 1024 || new_port > 65534)
        return util.show_alert_top_tm(this.$t("invalid-ports"));
      cpp.start_socks(new_port);
    },
    chg_http_port() {
      let new_port = prompt(this.$t("new-port"), util.http_port());
      if (new_port === null) {
        return; //break out of the function early
      }
      new_port = parseInt(new_port);
      if (isNaN(new_port))
        return util.show_alert_top_tm(this.$t("invalid-format"));
      if (new_port <= 1024 || new_port > 65534)
        return util.show_alert_top_tm(this.$t("invalid-ports"));
      cpp.restart(new_port);
    },
    req_whitelist() {
      cpp.isIgnoringBatteryOptimizations(
        responce => {
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
            console.log(
              "Application already Ignoring Battery Optimizations, show menu"
            );
            cpp.requestOptimizationsMenu(() => {}, () => {});
          }
        },
        error => {
          console.error("IsIgnoringBatteryOptimizations Error" + error);
        }
      );
    },
    refresh_qr() {
      const qr = new QRious({
        size: 200,
        element: document.getElementById("addr_qr"),
        value: this.address
      });
    },
    gen_qr_address() {
      networkinterface.getWiFiIPAddress(
        info => {
          this.wifi_ip = info.ip;
          this.refresh_qr();
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
.chg-port {
  background-color: aquamarine;
}
.socks {
  display: inline;
}
p {
  margin: 0.7em 1.7em;
  text-align: left;
}
.svr_addr {
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
i {
  color: green;
}
</style>
