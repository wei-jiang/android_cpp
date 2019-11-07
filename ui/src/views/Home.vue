<template>
  <div class="home">
    <div v-show="wifi_ip">
      <h3>
        {{address}}
      </h3>
      <!-- can not use v-if, cause not be able to get element -->
      <canvas id="addr_qr"></canvas>
      <div class="api-panel">
        <a :href="api_url" target="_blank">后台api接口说明</a>
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

export default {
  name: "Home",
  props: {
    msg: String
  },
  created: async function() {
    this.port = util.http_port() + 1;
  },
  destroyed() {

  },
  mounted() {
    this.gen_qr_address();
  },
  data() {
    return {
      wifi_ip: "",
      port: 57001,
    };
  },
  computed: {
    address() {
      return `http://${this.wifi_ip}:${this.port}`;
    },
    api_url(){
      return `http://${this.wifi_ip}:${this.port-1}/api.txt`;
    }
  },
  methods: {
    refresh_qr() {
      const qr = new QRious({
        size: 200,
        element: document.getElementById("addr_qr"),
        value: this.address
      });
    },
    async gen_qr_address() {
      try {
        this.wifi_ip = await util.get_ip();
        this.refresh_qr();
      } catch (error) {
        
      }
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

p {
  margin: 0.7em 1.7em;
  
}
.api-panel{
  margin: 1.1em 1em;
  font-size: 1.3rem;
}
.home {
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
