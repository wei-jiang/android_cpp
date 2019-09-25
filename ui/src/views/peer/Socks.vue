<template>
  <div class="proxy">
    socks5 代理信息
    <!-- <div v-for="(v, k) in proxy" class="server">
      <div class="bubble">{{v.total}}</div>
      <div class="addr">{{k}}</div>
      <div class="wc" @click.stop="chat_on(k)">
        <i class="small material-icons green">chat</i>
        &nbsp;&nbsp;
      </div>
    </div> -->

  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "proxy",
  props: {
    msg: String
  },
  created: async function() {
    this.$root.$on("ss_changed", this.refresh);
    this.$root.$on("online_count", this.refresh);
  },
  destroyed() {
    this.$root.$off("ss_changed", this.refresh);
    this.$root.$off("online_count", this.refresh);
  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      proxy: [],

    };
  },
  computed: {
    address() {
      
    }
  },
  methods: {
    chat_on(addr) {
      this.$router.push({ name: 'world', query: { addr } } );
    },

    refresh(){
      this.proxy = proxy;
    },

    remove(s) {

    },
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

.server{
  background-color: rgb(192, 206, 218);
  border: 1px lightcyan inset;
  width: 100%;
  display: flex;
  margin: 0.7em 0.5em 0.5em 0;
  align-items: center;
}
.addr{
  margin-left: 2em; 
}
.wc {
  margin-left: auto;
}
.bubble{
  background: #666;
  border-radius: 50%;
  color: #fff;
  display: inline-block;
  font-weight: bold;
  line-height: 2.7em;
  text-align: center;
  min-width: 2.7em; 
}

.proxy {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}



</style>
