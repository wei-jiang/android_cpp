<template>
  <div class="my">
    <nav>
      <div class="settings selected" @click.prevent="switch_to('settings', $event)">{{$t('settings')}}</div>
      <div class="friends" @click.prevent="switch_to('friends', $event)">{{$t('friends')}}</div>
      <div class="blacklist" @click.prevent="switch_to('blacklist', $event)">{{$t('blacklist')}}</div>
    </nav>
    <div class="main">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "My",
  props: {
    msg: String
  },
  created: async function() {

  },
  destroyed() {

  },
  watch: {
    '$route' (to, from) {
      $(`.my > nav > div`).removeClass('selected')
      $(`.my > nav > .${to.name}`).addClass('selected')
    }
  },
  mounted() {
  
  },
  data() {
    return {


    };
  },
  computed: {
    address() {
      return ``;
    }
  },
  methods: {
    switch_to(tab, e) {
      this.$router.replace({name: tab})
    },

  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.main {
  width: 100%;
  height: calc(100vh - 6.5rem);
  overflow-y: auto;
}

nav {
  display: flex;
  flex-wrap: nowrap;
  margin: 0.5em 0;
  width: 100%;
}
nav > div {
  background-color: aquamarine;
  flex: 1;
  font-size: 1.2em;
  border: 2px outset;
  border-radius: 0.7em;
}
.selected {
  border: 2px inset;
}
.my {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}

</style>
