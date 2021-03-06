import Vue from 'vue'
import Router from 'vue-router'

// import Browse from '@/public/views/Browse.vue'
import Store from '@/views/Store.vue';
import Upload from '@/public/views/Upload.vue'
import UpHome from '@/public/views/UpHome.vue'
import All from '@/views/fs/All.vue';
import Folder from '@/views/fs/Folder.vue';
import Image from '@/views/fs/Image.vue';
import Audio from '@/views/fs/Audio.vue';
import Video from '@/views/fs/Video.vue';
Vue.use(Router)

export default new Router({
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
        // ...other sub routes
      ]
    },
    {
      path: '/upload',
      name: 'upload',
      component: Upload
    },
    {
      path: '/uphome',
      name: 'uphome',
      component: UpHome
    },
  ]
})
