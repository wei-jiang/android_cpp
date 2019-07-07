import Vue from 'vue';
import Router from 'vue-router';
import Home from '@/views/Home.vue';

import All from '@/views/fs/All.vue';
import Folder from '@/views/fs/Folder.vue';
import Image from '@/views/fs/Image.vue';
import Audio from '@/views/fs/Audio.vue';
import Video from '@/views/fs/Video.vue';
import MultiSel from '@/views/fs/MultiSel.vue';
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
        { path: 'image', name: 'image', component: Image },
        { path: 'audio', name: 'audio', component: Audio },
        { path: 'video', name: 'video', component: Video },
        { path: 'multi-sel', name: 'multi-sel', component: MultiSel },
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
    },
    {
      path: '/help',
      name: 'help',
      component: () => import(/* webpackChunkName: "intranet" */ './views/Help.vue')
    },
  ]
});
