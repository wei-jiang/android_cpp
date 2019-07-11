<template>
  <div id="app">
    
    <header>{{title}}{{sub_title}}</header>
    <nav class="menu" id="main-menu">
      <div class="menu-toggle" @click="toggle_menu">主菜单</div>
      <div class="menu-dropdown">
        <ul class="nav-menu">
          <li>
            <a class="mb" @click="to_page('/help', '帮助', $event)"><i class="material-icons">help_outline</i>&nbsp;&nbsp;帮助</a>
          </li>

        </ul>
      </div>
    </nav>
    <div class="content">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    <footer>
      <div class="mb selected" @click="to_page('/', `文件管理`, $event)"><i class="material-icons">store</i>文件管理</div>
      <div class="mb" @click="to_page('/intranet', '服务地址', $event)"><i class="material-icons">wifi</i>服务地址</div>

    </footer>
  </div>
</template>
<script>

import util from "./common/util";
import cfg from "./common/config";
import adb from "./db";
import ws from "./ws";
export default {
  name: "App",
  created: function() {
    this.$root.$on("sub_title_chg", this.sub_title_chg);
    document.addEventListener("deviceready", this.deviceready, false);
    window.vm = this.$root;
  },
  beforeDestroy() {},
  destroyed() {
    this.$root.$off("sub_title_chg", this.sub_title_chg);
    document.removeEventListener("deviceready", this.deviceready, false);
  },
  mounted() {
    console.log(`navigator.language = ${navigator.language}`);
    if(navigator.language === 'zh-CN'){
      i18n.locale = 'zh'
    }
    // "navigator.language = zh-CN"
    // "navigator.language = en-US"
  },
  data() {
    return {
      title: '文件管理',
      sub: ''
    };
  },
  computed: {
    sub_title() {
      return this.sub ? `(${this.sub})`:"";
    }
  },
  methods: {
    toggle_menu() {
      const menu = document.getElementById("main-menu");
      menu.classList.toggle("is-open");
    },
    sub_title_chg(sub){
      this.sub = sub;
    },
    to_page(name, title, e) {      
      // router.replace(location, onComplete?, onAbort?)
      this.$router.replace(name, ()=>{
        this.title = title;
        $(".mb").removeClass("selected");
        $(e.target).addClass("selected");
        this.sub = '';
        $("#main-menu").removeClass("is-open");
      });
      
    },
    async deviceready() {
      window.db = await adb;
      window.cli_id = `${device.platform}-${device.manufacturer}-${device.model}-${device.uuid}`;
      try {
        window.cli_id = util.md5(window.cli_id);
      } catch (err) {
        console.log(err)
      }
      cpp.start( cfg.svr_port, () => {ws.init()}, err => {} );     
    }
  }
};
</script>
<style>
@import './assets/font.css';
:root {
  box-sizing: border-box;
  font-size: calc(1vw + 0.9em);
}
@media (min-width: 35em) {

}
@media (min-width: 50em) {
  :root {
    font-size: 1.25em;
  }
}
/* *,
*::before,
*::after {
  box-sizing: inherit;
} */
body {
  margin: 0;
  font-family: Helvetica, Arial, sans-serif;
}
#app {
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
  background-color: lightgray;
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
  height: calc(100vh - 4.0rem);
  overflow-y: auto;
}
/* menu */
.menu {
  position: relative;
}
.menu-toggle {
  position: absolute;
  top: -0.75em;
  left: 0;
  border: 0;
  background-color: transparent;
  font-size: 2.5em;
  width: 1em;
  height: 1em;
  line-height: 0.4;
  text-indent: 5em;
  white-space: nowrap;
  overflow: hidden;
}
.menu-toggle::after {
  position: absolute;
  top: 0.2em;
  left: 0.2em;
  display: block;
  content: "\2261";
  text-indent: 0;
}
.menu-dropdown {
  display: none;
  position: absolute;
  left: 0.7em;
  /* width: 40%; */
  padding: 0 1.7em;
  margin: 0;
  z-index: 79;
}
.menu.is-open .menu-dropdown {
  display: block;
}
.nav-menu {
  margin: 0;
  padding-left: 0;
  border: 1px solid #ccc;
  list-style: none;
  background-color: rgb(66, 60, 60);
  color: #fff;
}
.nav-menu > li + li {
  border-top: 1px solid #ccc;
}
.nav-menu > li > a {
  display: block;
  padding: 0.8em 1em;
  color: #fff;
  font-weight: normal;
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
button {
  min-height: max-content;
}
i.material-icons{
  vertical-align: sub; 
  pointer-events: none;
  /* border: 1px solid; */
}


</style>
