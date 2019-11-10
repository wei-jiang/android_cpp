import Vue from 'vue';
import Router from 'vue-router';
import Store from '@/views/Store.vue';

import All from '@/views/fs/All.vue';
import Folder from '@/views/fs/Folder.vue';
import Image from '@/views/fs/Image.vue';
import Audio from '@/views/fs/Audio.vue';
import Video from '@/views/fs/Video.vue';
import MultiSel from '@/views/fs/MultiSel.vue';

import My from '@/views/My.vue';
import Settings from '@/views/my/Settings.vue';
import Friends from '@/views/my/Friends.vue';
import Blacklist from '@/views/my/Blacklist.vue';

import Connected from '@/views/peer/Connected.vue';
import Nearby from '@/views/peer/Nearby.vue';
import Sss from '@/views/peer/Sss.vue';
import World from '@/views/peer/World.vue';
import Socks from '@/views/peer/Socks.vue';
import Phome from '@/views/peer/Home.vue';
Vue.use(Router);

export default new Router({
  // mode: 'hash',
  routes: [
    {
      path: '/',
      name: 'store',
      component: Store,
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
      path: '/outer',
      name: 'outer',
      component: () => import('./views/Outer.vue')
    },
    {
      path: '/peer-chat/:tp',
      props: true,
      name: 'peer-chat',
      component: () => import('./views/PeerChat.vue')
    },
    {
      path: '/peer',
      name: 'peer',
      component: () => import('./views/Peer.vue'),
      children: [
        { path: '', name: 'connected', component: Connected },
        { path: 'nearby', name: 'nearby', component: Nearby },
        { path: 'sss', name: 'sss', component: Sss },
        { path: 'world', name: 'world', component: World },
        { path: 'socks', name: 'socks', component: Socks },      
        { path: '/phome/:url', name: 'phome', component: Phome, props: true, },     
      ]
    },
    {
      path: '/my',
      name: 'my',
      component: My,
      children: [
        { path: '', name: 'settings', component: Settings },
        { path: 'friends', name: 'friends', component: Friends },
        { path: 'blacklist', name: 'blacklist', component: Blacklist },
      ]
    },
    {
      path: '/help',
      name: 'help',
      component: () => import(/* webpackChunkName: "help" */ './views/Help.vue')
    },
    {
      path: '/help-en',
      name: 'help-en',
      component: () => import(/* webpackChunkName: "help-en" */ './views/Help_en.vue')
    },
    {
      path: '/home',
      name: 'home',
      component: () => import('./views/Home.vue')
    },
  ]
});
