<template>
  <div class="folder">
    
    <!-- {{ $t('message') }} -->
    <div class="fi" v-for="f in files" @click="open_folder(f)">
      <div v-if="f.is_dir">
        <i class="small material-icons">folder</i>
      </div>
      <div class="file-desc">
        <div>{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
        </div>
      </div>
    </div>
    <!-- for move folder operation -->
    <div id="mfo">
      <div class="mfo-header">
        点此拖动
      </div>
      <div>
        <button @click="cancel_move">取消</button>
        <button @click="confirm_move">确定</button>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import cfg from "@/common/config";
import util from "@/common/util";
import ws from "@/ws";
export default {
  name: "folder",
  created: function() {
    this.$root.$on("update_file_list", this.update_file_list);
  },
  destroyed() {
    this.$root.$off("update_file_list", this.update_file_list);
  },
  mounted() {
    this.files = this.filter_folder(g.files);
    this.draggie = new Draggabilly('#mfo', {
      containment: '.folder', 
      handle: '.mfo-header'
    });
  },
  data() {
    return {
      files: []
    };
  },
  computed: {
    store_url() {
      return '';
    }
  },
  methods: {
    cancel_move(){
      this.$root.$emit('cancel_move', '');
    },
    confirm_move(){
      this.$root.$emit('confirm_move', '');
    },
    filter_folder(files){
      return _.filter(files, f=>f.is_dir);
    },
    open_folder(f) {
        this.$root.$emit("enter_dir", f.name);
    },
    update_file_list(files) {
      this.files = this.filter_folder(files);
    },
  }
};
</script>
<style scoped >
.folder{
  min-height: 100%;
  
}
#mfo{
  background-color: gray;
  font-size: 1.7rem;
}
button{
  font-size: 1.5rem;
}
.mfo-header{
  background-color: rgb(88, 88, 88);
}
.file-desc {
  flex: 1;
  margin: 0 0.4em;
}

.file-time {
  display: flex;
  font-size: 0.7rem;
}

.fi {
  display: flex;
  align-items: center;
  text-align: left;
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}

</style>
