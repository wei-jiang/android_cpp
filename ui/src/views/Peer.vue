<template>
  <div class="peer">
    <!-- <nav>
      <div class="selected" @click.prevent="switch_to('connected', $event)">{{$t('connected')}}</div>
      <div @click.prevent="switch_to('nearby', $event)">{{$t('nearby')}}</div>
      <div @click.prevent="switch_to('world', $event)">{{$t('world')}}</div>
    </nav> -->
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
  name: "Peer",
  props: {
    msg: String
  },
  created: async function() {

  },
  destroyed() {

  },
  mounted() {

  },
  data() {
    return {

    };
  },
  computed: {
    address() {

    }
  },
  methods: {
    switch_to(tab, e) {
      this.$router.replace({name: tab})
      $("nav > div").removeClass("selected");
      $(e.target).addClass("selected");
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
}
.selected {
  border: 2px inset;
}
.peer {
  width: 100%;
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  overflow: hidden;
  font-weight: 700;
}


</style>
