<template>
  <div class="my">
    个人设置
  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "My",
  props: {
    msg: String
  },
  created: async function() {

  },
  destroyed() {

  },
  mounted() {
    this.sss = util.ss_addrs()
  },
  data() {
    return {
      sss: [],
      how_to: false
    };
  },
  computed: {
    address() {
      return `http://${this.wifi_ip}:${this.port}`;
    }
  },
  methods: {
    chg_socks_port_back(data) {

    },
    chg_http_port_back(data) {

    },
    
    
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
.my {
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
