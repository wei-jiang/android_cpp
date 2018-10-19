import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'

import 'noty/lib/noty.css'

// import adb from "./db";

window.$ = require('jquery')

Vue.config.productionTip = false
window.isCordovaApp = !!window.cordova;
window.vm = new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
