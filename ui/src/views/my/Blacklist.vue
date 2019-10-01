<template>
  <div class="blacklist">
    <div class="blocked" v-for="b in block_list">
      <div>
        <i class="small material-icons">block</i>
      </div>
      <div class="peer-info" @click="show_detail">
        <div>{{title(b)}}<i class=" material-icons">info_outline</i></div>       
        <div class="b-info">
          <img v-if="b.avatar" :src="b.avatar">
          <div v-if="b.signature">{{b.signature}}</div>
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
    this.$root.$on("refresh_block_list", this.refresh);
  },
  destroyed() {
    this.$root.$off("refresh_block_list", this.refresh);
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
    title(p){
      return p.nickname || util.truncate(p.id, 16)
    },
    refresh(){
      this.block_list = db.blacklist.find({});
    },
    show_detail(e){
      $(e.currentTarget).find(`.b-info`).toggle();
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
.b-info{
  display: none;
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
