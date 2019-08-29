<template>
  <div class="friends">
    nearby channel
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";

export default {
  name: "friends",
  created: function() {
    this.$root.$on("add_friend", this.refresh);
  },
  destroyed() {
    this.$root.$off("add_friend", this.refresh);
  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      friend_list: []
    };
  },
  computed: {

  },
  methods: {
    refresh(){
      this.friend_list = db.friends.find({}).map(b=>{
        b.show_type = false;
        return b;
      });
    },
    show_detail(b){
      b.show_type = !b.show_type
    },
    remove(b){
      navigator.notification.confirm(
          `[${b.nickname}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              db.friends.remove(b);
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
.friends{
  width: 100%;
  min-height: 97%;

}
.red {
  color: red;
}

.favorite {
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
