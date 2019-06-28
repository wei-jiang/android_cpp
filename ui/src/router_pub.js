import Vue from 'vue'
import Router from 'vue-router'


import Browse from './public/views/Browse.vue'
import Upload from './public/views/Upload.vue'
Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'browse',
      component: Browse
    },
    {
      path: '/upload',
      name: 'upload',
      component: Upload
    },
    
  ]
})
