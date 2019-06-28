import Vue from 'vue'
import App from './App.vue'
import router from './router'
import './registerServiceWorker'
import i18n from './i18n'
import 'noty/lib/noty.css'
window.$ = require('jquery')
import Pub from './Pub.vue'
import router_pub from './router_pub'

Vue.config.productionTip = false

window.isCordovaApp = !!window.cordova;
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

