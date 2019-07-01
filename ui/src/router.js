import Vue from 'vue';
import Router from 'vue-router';
import Home from './views/Home.vue';

import All from './views/fs/All.vue';
import Folder from './views/fs/Folder.vue';
Vue.use(Router);

export default new Router({
  // mode: 'hash',
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home,
      children: [
        { path: '', name: 'all', component: All },
        { path: 'folder', name: 'folder', component: Folder },
        // ...other sub routes
      ]
    },
    {
      path: '/intranet',
      name: 'intranet',
      // route level code-splitting
      // this generates a separate chunk (intranet.[hash].js) for this route
      // which is lazy-loaded when the route is visited.
      component: () => import(/* webpackChunkName: "intranet" */ './views/Intranet.vue')
    }
  ]
});
