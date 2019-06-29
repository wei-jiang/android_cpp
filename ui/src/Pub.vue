<template>
  <div id="pub">

    <header>{{title}}</header>
    <div class="content">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    <footer>
      <div class="mb selected" @click="to_page('/', `浏览文件`, $event)">浏览文件</div>
      <div class="mb" @click="to_page('/upload', '上传文件', $event)">上传文件</div>
    </footer>
    
  </div>
</template>
<script>
import util from "./common/util";
import uuidv1 from 'uuid/v1'
import ws from "./ws";
window.cli_id = localStorage.getItem('cli_id')
if(!window.cli_id){
  window.cli_id = uuidv1()
  localStorage.setItem('cli_id', window.cli_id)
}
export default {
  name: "Pub",
  created: async function() {
    window.vm = this.$root;
    ws.init()
  },
  destroyed() {

  },
  mounted() {

  },
  data() {
    return {
      title: ""
    };
  },
  computed: {
    caption() {

      return "【飘云软件】";
    }
  },
  methods: {
    
    to_page(name, title, e) {
      this.title = title;
      this.$router.replace(name);
      $(".mb").removeClass("selected");
      $(e.target).addClass("selected");
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

  
</style>
