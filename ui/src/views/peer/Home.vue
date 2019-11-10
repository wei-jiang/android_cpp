<template>
  <div class="phome">
    <div class="title">
      <div @click="$router.go(-1)" class="small material-icons">keyboard_backspace</div>
      <div class="url">
        <div>{{url}}</div>
      </div>
      <div @click.stop="refresh()">
        <i class="small material-icons">refresh</i>
      </div>
    </div>
    <div class="tip">提示：以上地址可在局域网内用其它浏览器打开</div>
    <hr>
    <iframe :src="url"></iframe>
  </div>
</template>

<script>

import util from "@/common/util";

export default {
  name: "phome",
  props: {
    url: String
  },
  created: async function() {

  },
  destroyed() {

  },
  mounted() {

  },
  data() {
    return {

    };
  },
  computed: {
    address() {
      
    }
  },
  methods: {
    refresh() {
      document.querySelector('iframe').contentWindow.location.reload();
    },

  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.phome {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
  height: calc(100vh - 6.0rem);
}
.title {
  width: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
}
.url{
  font-size: 1.1rem;
  flex: 1;
}
.tip{
  font-size: 0.8rem;
  color: darkgray;
}
hr{
  width: 100%;
}
iframe{
  width: 100%;
  height: 100%;
  border:none;
}
</style>
