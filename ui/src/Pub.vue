<template>
  <div id="pub" @click="enter_full">

    <header>{{title}}</header>
    <div class="content">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    <footer>
      <div class="all" @click="to_page('/', $t('browse-file'), $event)">{{$t('browse-file')}}</div>
      <div class="upload" @click="to_page('/upload', $t('upload-file'), $event)">{{$t('upload-file')}}</div>
      <div class="uphome" @click="to_page('/uphome', $t('upload-home'), $event)">{{$t('upload-home')}}</div>
    </footer>
    
  </div>
</template>
<script>
import util from "@/common/util";
import uuidv1 from 'uuid/v1'
// import screenfull from 'screenfull'
import WS from "@/ws";
import vhCheck from 'vh-check'
const test = vhCheck()
window.cli_id = localStorage.getItem('cli_id')
if(!window.cli_id){
  window.cli_id = uuidv1()
  localStorage.setItem('cli_id', window.cli_id)
}
export default {
  name: "Pub",
  created: async function() {
    window.vm = this.$root;
    window.ws = new WS();
    ws.init()
  },
  destroyed() {

  },
  watch: {
    '$route': function(to, from) {
      // console.log(`to.name=${to.name}`)
      this.push_tab(to.name);
    }
  },
  mounted() {
    if(navigator.language !== 'zh-CN'){
      document.title = 'android resource server';
    }
    // console.log(this.$route.name)
    this.push_tab(this.$route.name);
  },
  data() {
    return {
      title: this.$t("file-mgr"),
    };
  },
  computed: {
    caption() {
      return "";
    }
  },
  methods: {
    push_tab(name){
      $(`footer > div`).removeClass('selected')
      $(`footer > .${name}`).addClass('selected')
    },
    enter_full(){
      // if (screenfull.enabled && !screenfull.isFullscreen) {
      //   screenfull.request();
      //   console.log('enter fullscreen')
      // } else {
      //   console.log('not support fullscreen')
      // }     
    },

    to_page(name, title, e) {
      this.title = title;
      this.$router.replace(name);
    },

  }
};
</script>
<style>
:root {
  box-sizing: border-box;
  font-size: calc(1vw + 0.9em);
}
*,
*::before,
*::after {
  box-sizing: inherit;
}
body {
  margin: 0;
  font-family: Helvetica, Arial, sans-serif;
}
#pub {
  font-family: "Avenir", Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
header {
  height: 2rem;
  line-height: 2em;
  text-align: center;
  background-color: rgb(175, 245, 245);
}
footer {
  /* bottom: 0;
  position: fixed; */
  background-color: lightgray;
}
.selected {
  border: 2px inset;
}
footer > div {
  /* width: 4em; */
  border: 2px outset;
  flex: 1;
  /* border-radius: 0.9em; */
  background-color: lightblue;
  /* box-shadow: 5px 10px #888888; */
}

footer {
  display: flex;
  text-align: center;
  min-width: 100%;
  height: 2rem;
  line-height: 2em;
  justify-content: space-between;
}
.content {
  display: flex;
  flex-flow: column;
  /* background-image: url("./assets/bg.jpg"); */
  background-color: lightcyan;
  background-position: center;
  background-repeat: no-repeat;
  background-size: cover;
  height: calc(100vh - 4rem);
  height: calc(100vh - var(--vh-offset, 0px) - 4rem);
  overflow-y: auto;
}

/* -------------------------------------------------------------------------------- */
/* Tooltip container */
.tooltip {
  position: relative;
  display: inline-block;
  border-bottom: 1px dotted black; /* If you want dots under the hoverable text */
}

/* Tooltip text */
.tooltip .tooltiptext {
  /* visibility: hidden; */
  display: none;
  /* width: 7rem; */
  background-color: black;
  color: #fff;
  text-align: left;
  padding: 5px 0;
  border-radius: 6px;
  /* Position the tooltip text  */
  top: 100%;
  left: 20%;
  margin-left: -60px; /* Use half of the width (120/2 = 60), to center the tooltip */

  position: absolute;
  z-index: 1;
}
.btn {
  width: 30%;
  font-size: 1.1rem;
  margin: 1.4em;
  /* background-color: bisque; */
  border-radius: 0.9em;
}
.btn-ok {
  background-color: aquamarine;
  float: left;
}
.btn-cancel {
  background-color: rgb(245, 108, 108);
  float: right;
}
.noty_text {
  font-size: 1.2rem;
  text-align: center;
}
button{
  min-height: max-content;
}
/* Show the tooltip text when you mouse over the tooltip container */
/* .tooltip:hover .tooltiptext {
  visibility: visible;
} */
.pad-bottom{
  min-height: var(--vh-offset, 0px);
}
.yellow{
  color: rgb(199, 173, 87);
}
.green{
  color: green;
}
</style>
