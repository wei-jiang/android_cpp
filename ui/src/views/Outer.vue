<template>
  <div class="outer">
    <div v-for="s in sss" class="server">
      <input v-model="s.addr" />
      <div @click="enable_svr_or_not(s)">
        <i v-if="s.enabled" class="small material-icons">check_box</i>
        <i v-else class="small material-icons">check_box_outline_blank</i>
      </div>
      <div>&nbsp;&nbsp;&nbsp;</div>
      <div @click="remove(s)"><i class="small material-icons">delete</i></div>
    </div>
    <button @click="add_server"><i class="material-icons">add</i>&nbsp;{{$t('add')}}</button>
    <h3 @click.prevent="how_to = !how_to">{{$t('how-to-setup-ss')}}&#x2753;</h3>
    <div v-show="how_to" @click.prevent="how_to = !how_to">
      {{$t('how-to-setup-ss-content')}}
      <a href="https://github.com/novice79/ss.git" target="_blank">https://github.com/novice79/ss.git</a>
    </div>
  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "Outer",
  props: {
    msg: String
  },
  created: async function() {

  },
  destroyed() {

  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      sss: [],
      how_to: false
    };
  },
  computed: {
    address() {
      
    }
  },
  methods: {
    add_server(){
      let addr = prompt(this.$t('svr-addr'), '');
      if(!addr) return;
      // check addr format
      if( !util.check_addr(addr) ) return util.show_alert_top_tm( this.$t('invalid-format') )
      db.ss.insert({ addr, enabled: true });
      this.refresh();
    },
    refresh(){
      this.sss = util.ss_addrs()
    },
    enable_svr_or_not(s) {
      let title;
      if(s.enabled){
        title = this.$t('confirm-disable')
      } else {
        title = this.$t('confirm-enable')
      }
      navigator.notification.confirm(
          `[${s.addr}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              s.enabled = !s.enabled;
              db.ss.update(s);
              if(s.enabled){

              }else{

              }
            }
          },            
          title,           // title
          [this.$t('ok'), this.$t('cancel')]     // buttonLabels
      );
      
    },
    remove(s) {
      navigator.notification.confirm(
          `[${s.addr}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              db.ss.remove(s);
              this.refresh();
            }
          },            
          this.$t('confirm-del'),           // title
          [this.$t('ok'), this.$t('cancel')]     // buttonLabels
      );
    },
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
canvas {
  margin: 0.3em auto;
  width: 200px;
  height: 200px;
}
.server{
  display: flex;
  margin: 0.7em 0.5em;
}
input {
  /* flex: 1; */
  /* this necessary */
  min-width: 5em;
  width: 100%;
  flex-grow: 1;
  font-size: 1.3rem;
}
p {
  margin: 0.7em 1.7em;
  text-align: left;
}

.outer {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}
button {
  width: 80%;
  border-radius: 1em;
  font-size: 1.2em;
  /* padding: 0 1em; */
  margin: 1.0em 0.5em;
}


</style>
