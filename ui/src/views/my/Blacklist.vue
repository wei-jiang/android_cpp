<template>
  <div class="blacklist">
    <div class="blocked" v-for="b in block_list">
      <div>
        <i class="small material-icons">block</i>
      </div>
      <div class="peer-info" @click="show_detail(b)">
        <div>{{b.nickname}}<i class=" material-icons">info_outline</i></div>       
        <div v-if="b.shown">
          <img :src="b.avatar">
          <div>{{b.signature}}</div>
        </div>
      </div>
      <div @click.stop="remove(b)">
        <i class="red small material-icons">remove_circle</i>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";

export default {
  name: "blacklist",
  created: function() {
    
  },
  destroyed() {
   
  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      block_list: []
    };
  },
  computed: {

  },
  methods: {
    refresh(){
      this.block_list = db.blacklist.find({}).map(b=>{
        b.shown = false;
        return b;
      });
    },
    show_detail(b){
      b.shown = !b.shown
      // console.log(`b.shown=${b.shown}`)
    },
    remove(b){
      navigator.notification.confirm(
          `[${b.nickname}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              db.blacklist.remove(b);
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
<style scoped >
.blacklist{
  width: 100%;
  min-height: 97%;

}
.red {
  color: red;
}

.blocked {
  display: flex;
  /* align-items: center; */
  /* text-align: left; */
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
img{
  width: 90%;
}
.peer-info{
  flex: 1;
}

</style>
