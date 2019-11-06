<template>
  <div id="app">
    <header>{{title}}{{sub_title}}</header>
    <nav class="menu" id="main-menu">
      <div class="menu-toggle" @click="toggle_menu">{{ $t('main-menu') }}</div>
      <div class="menu-dropdown">
        <ul class="nav-menu">
          <li>
            <!-- crop_free phone_android center_focus_strong center_focus_weak-->
            <a class="mb" @click="scan_qr()">
              <i class="material-icons">center_focus_weak</i>
              &nbsp;&nbsp;{{$t('scan')}}
            </a>
          </li>
          <li>
            <a class="mb" @click="to_page('/outer', $t('outer'), $event)">
              <i class="material-icons">language</i>
              &nbsp;&nbsp;{{$t('outer')}}
            </a>
          </li>
          <li>
            <a class="mb" @click="to_page('/peer', $t('peer'), $event)">
              <i class="material-icons">people</i>
              &nbsp;&nbsp;{{$t('peer')}}
            </a>
          </li>
          <li>
            <a class="mb" @click="to_page('/my', $t('my'), $event)">
              <i class="material-icons">person</i>
              &nbsp;&nbsp;{{$t('my')}}
            </a>
          </li>
          <li>
            <a class="mb" @click="to_page('/help', $t('help'), $event)">
              <i class="material-icons">help_outline</i>
              &nbsp;&nbsp;{{$t('help')}}
            </a>
          </li>
          <!-- <li>
            <a class="mb" @click="test">&nbsp;&nbsp;test</a>
          </li> -->
        </ul>
      </div>
    </nav>
    <div class="content">
      <keep-alive>
        <router-view />
      </keep-alive>
    </div>
    <footer>
      <div class="mb selected" @click="to_page('/', $t('file-mgr'), $event)">
        <i class="material-icons">store</i>
        {{$t('file-mgr')}}
      </div>
      <div class="mb" @click="show_svr_addr($event)">
        <i class="material-icons">wifi</i>
        {{$t('svr-addr')}}
      </div>
      <div class="mb" @click="to_page('/home', $t('home-addr'), $event)">
        <i class="material-icons">home</i>
        {{$t('home-addr')}}
        
      </div>
    </footer>
  </div>
</template>
<script>
import Peer from "simple-peer";
import util from "@/common/util";
import busi from "@/common/busi";
import WS from "@/ws";
import WsTunnel from "@/ws_tunnel";

export default {
  name: "App",
  created: function() {
    window.vm = this.$root;
    this.$root.$on("sub_title_chg", this.sub_title_chg);
    this.$root.$on("http_ready", this.http_ready);

  },
  beforeDestroy() {},
  destroyed() {
    this.$root.$off("sub_title_chg", this.sub_title_chg);
    this.$root.$off("http_ready", this.http_ready);

  },
  async mounted() {
    await util.wait_deviceready();
    this.init_native();
  },
  data() {
    return {
      title: this.$t("file-mgr"),
      sub: ""
    };
  },
  computed: {
    sub_title() {
      return this.sub ? `(${this.sub})` : "";
    }
  },
  methods: {
    async test() {
      // cpp.showInterstitialAd();
      // cpp.showBanner(()=>{
      //   console.log('show banner success')
      // }, ()=>{
      //   console.log('show banner faled')
      // });
      // cpp.restart(57001);
      // cpp.echo('jiang', res=>alert(res) )
      // if (Peer.WEBRTC_SUPPORT) {
      //   // webrtc support!
      //   alert("webrtc support");
      // } else {
      //   // fallback
      //   alert("webrtc not support");
      // }
      
      const self = this;
      setTimeout(()=>{
        console.log("from cordova activity ..................");
        self.test()
      }, 2000)
      // this.toggle_menu();
    },

    cpp_noty(data) {
      // console.log("from C++++++++++++++ " + data + " +++++++++++++++C");
      try {
        data = JSON.parse(data);
        this.$root.$emit(data.cmd, data);
      } catch (err) {
        const hex_str = Buffer.from(data, "binary").toString("hex");
        console.log(`hex_str=${hex_str}`);
      }
    },
    async http_ready(data) {
      cpp.start_socks(util.socks_port());
      window.ws = new WS();
      ws.init();
      window.ws_tunnel = new WsTunnel();     
      try {
        const local_ip = await util.get_ip();
        const socks_port = parseInt( util.socks_port() );
        await util.write_socks_pac(local_ip, socks_port);
        util.write_remote_socks_pac(local_ip, socks_port+100);
        window.remote_socks_addr = `${local_ip}:${socks_port+100}`;
        window.http_addr = `http://${local_ip}:${util.http_port()}/`;
      } catch (error) {
        console.log(`no wifi ip, so skip writing pac files`);
      }      
      // cpp.requestDrawOverlays(ret=>{
      //   console.log(`cpp.requestDrawOverlays, ret=${ret}`)
      //   if(ret == 1){
      //     cpp.mount_webview(`http://127.0.0.1:${data.port}/service.html`);
      //   }
      // })
      
    },
    scan_qr() {
      cpp.scan_by_camera(
        data => {
          // util.show_alert_top(JSON.stringify(data) );
          if (!data.cancelled) {
            const qr_code = data.text;
            console.log(`qr_code=${qr_code}`);
            if (qr_code.startsWith("http")) {
              window.open(qr_code);
            } else {
              navigator.notification.confirm(
                qr_code, // message
                i => {
                  // the index uses one-based indexing, so the value is 1, 2, 3, etc.
                  if (i == 1) {
                    cpp.copyText(qr_code, () => {
                      util.show_alert_top_tm(this.$t("copy-done"));
                    });
                  }
                },
                this.$t("read-code-content"), // title
                [this.$t("copy"), this.$t("close")] // buttonLabels
              );
            }
          } else {
            console.log(`scan qr cancelled`);
          }
        },
        err => {
          util.show_alert_top(err);
        }
      );
      this.toggle_menu();
    },
    show_svr_addr(e) {
      this.to_page("/intranet", this.$t("svr-addr"), e);
      //disable ads for now
      // if(is_ads_tm){
      //   cpp.showInterstitialAd(()=>{
      //     util.restart_ads_tm();
      //   });
      // }
    },
    toggle_menu() {
      const menu = document.getElementById("main-menu");
      menu.classList.toggle("is-open");
    },

    sub_title_chg(sub) {
      this.sub = sub;
    },
    to_page(name, title, e) {
      // router.replace(location, onComplete?, onAbort?)
      this.$router.replace(name, () => {});
      this.title = title;
      $(".mb").removeClass("selected");
      $(e.target).addClass("selected");
      this.sub = "";
      $("#main-menu").removeClass("is-open");
    },
    init_native() {
      //disable ads for now
      // util.restart_ads_tm();
      window.cli_id = `${device.platform}-${device.manufacturer}-${device.model}-${device.uuid}`;
      try {
        // 32bit hashCode likely has conflict, so just use md5 hash string
        window.cli_id = util.md5(window.cli_id);
      } catch (err) {
        console.log(err);
      }
      busi.init();
      cpp.start(
        util.http_port(),
        () => {
          
        },
        err => {}
      );
      cpp.reg_cpp_cb(this.cpp_noty.bind(this));
      
    }
  }
};
</script>
<style>
@import "./assets/font.css";
@import "./assets/ol.css";
@import "./assets/common.css";
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
  height: calc(100vh - 4rem);
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

button {
  min-height: max-content;
}
i.material-icons {
  vertical-align: sub;
  pointer-events: none;
  /* border: 1px solid; */
}

.yellow-new {
  color: rgb(87, 67, 1);
}
.yellow {
  color: rgb(199, 173, 87);
}
.green {
  color: green;
}
</style>
