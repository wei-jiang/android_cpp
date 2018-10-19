<template>
  <div class="home">
    <img alt="Vue logo" src="../assets/logo.png">
    <ResMgr msg="Welcome to Your Vue.js App"/>
  </div>
</template>

<script>
// @ is an alias to /src
import ResMgr from '@/components/ResMgr.vue'

export default {
  name: 'home',
  components: {
    ResMgr
  }
}
</script>
