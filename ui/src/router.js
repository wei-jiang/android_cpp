import Vue from 'vue';
import Router from 'vue-router';
import Home from './views/Home.vue';

Vue.use(Router);

export default new Router({
  // mode: 'hash',
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home
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
