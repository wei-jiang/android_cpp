<template>
  <div class="player" @click.stop="hide_menu">

    <div class="main">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    <div class="player-menu" onclick="event.cancelBubble = true;"> 
      <i class="handle small material-icons">menu</i>
      <div class="player-dropdown">
        <div class="connected disabled" @click="switch_to('connected', $event)"><i class="material-icons">face</i>{{$t('connected')}}</div>
        <div class="nearby" @click="switch_to('nearby', $event)"><i class="material-icons">group</i>{{$t('nearby')}}</div>
        <div class="sss" @click="switch_to('sss', $event)"><i class="material-icons">language</i>{{$t('world')}}</div>
        <div class="socks" @click="switch_to('socks', $event)"><i class="material-icons">vpn_key</i>{{$t('proxy')}}</div>       
      </div>
    </div>
  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "Player",
  props: {
    msg: String
  },
  created: async function() {

  },
  destroyed() {

  },
  watch: {
    '$route' (to, from) {
      // console.log(`from=${JSON.stringify(from.name)}`)
      // console.log(`to=${JSON.stringify(to.name)}`)
      $(`.player-menu > div > div`).removeClass('disabled')
      $(`.player-dropdown > .${to.name}`).addClass('disabled')
    }
  },
  mounted() {
    this.draggie = new Draggabilly('.player-menu', {
      containment: '.player', 
      handle: '.handle'
    });
    this.draggie.on( 'dragStart', ()=>{
      // $(".player-menu").removeClass("is-open");
    });
    this.draggie.on( 'staticClick', ()=>{
      $(".player-menu").toggleClass("is-open");
      // console.log('staticClick');

    });
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
    hide_menu(){
      $(".player-menu").removeClass("is-open");
      $(".tooltiptext").hide();
    },
    switch_to(tab, e) {
      this.$router.replace({name: tab});
    },
    
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.main {
  width: 100%;
  height: calc(100vh - 4.0rem);
  overflow-y: auto;
}

.player {
  width: 100%;
  display: flex;
  /* justify-content: space-between; */
  position: relative;
  align-items: center;
  flex-flow: column;
  overflow: hidden;
  font-weight: 700;
}
.handle{
  padding: 0.35em;
}
.handle.material-icons{
  vertical-align: sub; 
  pointer-events: initial;
}
.player-menu{
  position: absolute;
  /* must use left/top, or it will stretch */
  left: 55%;
  top: 40%;
  border-radius: 2em;
  background-color: rgb(230, 227, 227);
}
.player-dropdown{
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  left: 40%;
  top: 80%;
  margin: 0;
}
.player-dropdown > div + div{
  border-top: 1px outset;
}
.player-dropdown > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
}
.player-menu.is-open > div {
  display: block;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}
div.disabled
{
  pointer-events: none;
  /* for "disabled" effect */
  opacity: 0.5;
  background: #CCC;
}

</style>
