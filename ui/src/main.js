import Vue from 'vue'

import App from '@/App.vue'
import router from '@/router'
import i18n from '@/i18n'
import 'noty/lib/noty.css'
import "@/assets/animate.css"
window.$ = require('jquery')
import Draggabilly from 'draggabilly'
window.Draggabilly = Draggabilly;
import Pub from '@/Pub.vue'
import router_pub from '@/router_pub'
import adb from "@/db";
Vue.config.productionTip = false
window.i18n = i18n;
window.isCordovaApp = !!window.cordova;

console.log(`navigator.language = ${navigator.language}`);
if(navigator.language === 'zh-CN'){
  i18n.locale = 'zh'
}
// "navigator.language = zh-CN"
// "navigator.language = en-US"
adb.then( db=>{
  window.db = db;
  if (window.isCordovaApp) { 
    new Vue({
      router,
      i18n,
      render: h => h(App)
    }).$mount('#app');    
  } else {  
    new Vue({
      router: router_pub,
      i18n,
      render: h => h(Pub)
    }).$mount('#app');
  }
})


