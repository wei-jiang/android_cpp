<template>
  <div class="home">
    <div class="cur">
      <div>{{cur_dir}}</div>
      <button class="return" @click="go_back" :disabled="dir.length <= 0">
        <i class="material-icons">keyboard_backspace</i>
      </button>
      <button class="new-folder" @click="create_folder">
        <i class="material-icons">create_new_folder</i>
      </button>
    </div>
    <div class="main">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    
    <!-- {{ $t('message') }} -->
    <div class="fs-menu" @click.stop="hide_menu"> 
      <i class="handle small material-icons">menu</i>
      <div class="fs-dropdown">
        <div @click="$root.$emit('sub_title_chg', '全部');$router.replace({name: 'all'})"><i class="material-icons">store</i>全部</div>
        <div @click="$root.$emit('sub_title_chg', '图片');$router.replace({name: 'image'})"><i class="material-icons">image</i>图片</div>
        <div @click="$root.$emit('sub_title_chg', '音频');$router.replace({name: 'audio'})"><i class="material-icons">audiotrack</i>音频</div>
        <div @click="$root.$emit('sub_title_chg', '视频');$router.replace({name: 'video'})"><i class="material-icons">ondemand_video</i>视频</div>
        
      </div>
      <div class="sort-dropdown">
        <div @click="sort_by(1)">
          按时间排序
          <i v-if="order_by_time=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="order_by_time=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div @click="sort_by(2)">
          按名称排序
          <i v-if="order_by_name=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="order_by_name=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div @click="sort_by(3)">
          按类型排序
          <i v-if="order_by_type=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="order_by_type=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import cfg from "@/common/config";
import util from "@/common/util";
import ws from "@/ws";
window.g = {};
export default {
  name: "home",
  created: function() {
    this.$root.$on("get_file_list", this.on_files_chg);
    this.$root.$on("enter_dir", this.on_enter_dir);
    this.$root.$on("create_dir", this.on_create_dir);
    this.$root.$on("refresh_file_list", this.on_refresh);
    this.$root.$on('move_to', this.move_to);
    this.$root.$on('cancel_move', this.cancel_move);
    this.$root.$on('confirm_move', this.confirm_move);
  },
  destroyed() {
    this.$root.$off("get_file_list", this.on_files_chg);
    this.$root.$off("enter_dir", this.on_enter_dir);
    this.$root.$off("create_dir", this.on_create_dir);
    this.$root.$off("refresh_file_list", this.on_refresh);
    this.$root.$off('move_to', this.move_to);
    this.$root.$off('cancel_move', this.cancel_move);
    this.$root.$off('confirm_move', this.confirm_move);
  },
  mounted() {
    this.draggie = new Draggabilly('.fs-menu', {
      containment: '.content', 
      handle: '.handle'
    });
    // this.draggie.on( 'dragStart', ()=>{
    //   $(".fs-menu").removeClass("is-open");
    // });
    this.draggie.on( 'staticClick', ()=>{
      $(".fs-menu").toggleClass("is-open");
      // console.log('staticClick');
    });
  },
  data() {
    return {
      order_by_time: '',
      order_by_name: '',
      order_by_type: '',
      pending_f: null,
      dir: [],
      original_dir: [],
    };
  },
  computed: {
    cur_dir() {
      const p = this.dir.join("/");
      if (p) return `/sdcard/mystore/${p}/`;
      return `/sdcard/mystore/`;
    }
  },
  methods: {
    sort_by(type){
      switch(type){
        case 1:{
          if(this.order_by_time == 'asc') this.order_by_time = 'desc';
          else if(this.order_by_time == 'desc') this.order_by_time = 'asc';
          else this.order_by_time = 'asc';
          // cause desc does not work as expected, so reverse it
          // console.log(`order_by_time: ${this.order_by_time}`)
          g.files = _.orderBy(g.files, ['time']);
          this.update_file_list( this.order_by_time == 'asc' ? g.files : _.reverse(g.files) );
          break;
        }
        case 2:{
          if(this.order_by_name == 'asc') this.order_by_name = 'desc';
          else this.order_by_name = 'asc';
          g.files = _.orderBy(g.files, ['name']); //always asc
          this.update_file_list( this.order_by_name == 'asc' ? g.files : _.reverse(g.files) )
          break;
        }
        case 3:{
          if(this.order_by_type == 'asc') this.order_by_type = 'desc';
          else this.order_by_type = 'asc';
          g.files = _.orderBy(g.files, ['type']); //always asc
          this.update_file_list( this.order_by_type == 'asc' ? g.files : _.reverse(g.files) )
          break;
        }
      }
    },
    cancel_move(){
      this.restore_before_move()
    },
    confirm_move(){
      let i = _.findIndex( g.files, f=> f.type == this.pending_f.type && f.name == this.pending_f.name );
      if(i >= 0) return util.show_alert_top_tm('文件(夹)已存在当前目录')
      // actually move and restore previous dir
      const cmd = {
        cmd: "rename_file",
        path: this.pending_f.path,
        new_name: this.cur_dir + this.pending_f.name
      };
      ws.send(JSON.stringify(cmd));
      this.restore_before_move()
    },
    restore_before_move(){
      this.dir = this.original_dir;
      this.$router.replace({name: 'all'});
      this.on_refresh();
    },
    move_to(f){
      this.original_dir = _.cloneDeep(this.dir);
      this.$router.replace('folder', ()=>{
        this.pending_f = f;
      });
    },
    hide_menu(){
      // $(".fs-menu").removeClass("is-open");
    },
    test(msg){
      // console.log('in test')
      alert(cli_id);

    },
    on_create_dir(data) {
      if (data.ret == 0) {
        const n = util.get_name_from_path(data.path);
        util.show_info_center_tm(`创建文件夹【${n}】成功`);
      } else {
        util.show_error_top(`创建文件夹失败：${data.msg}`);
      }
    },
    create_folder() {
      let name = prompt("新文件夹名称:", "新建文件夹");
      if(!name) return util.show_alert_top_tm('文件夹名称不能为空')
      name = name.replace(/[\n\r]/gm, "");
      if(!name) return util.show_alert_top_tm('文件夹名称不能为空')
      let i = _.findIndex( g.files, f=> f.type == 'dir' && f.name == name );
      if(i >= 0) return util.show_alert_top_tm('文件夹已存在')
      if (name) {
        const cmd = {
          cmd: "create_dir",
          path: `${this.cur_dir}${name}`
        };
        ws.send(JSON.stringify(cmd));
      }
    },
    go_back() {
      this.dir.pop();
      this.on_refresh();
    },
    on_enter_dir(name) {
      this.dir.push(name);
      this.on_refresh();
    },
    update_file_list(files){
      g.files = files;
      this.$root.$emit("update_file_list", g.files);
    },
    // ws onopen emit
    on_files_chg(data) {
      // console.log('on_files_chg: '+JSON.stringify(data) )
      this.update_file_list(data.files);
    },
    async on_refresh() {
      // console.log(`refresh ${this.cur_dir}`);
      try {
        const res = await util.post_local("get_files", {
          path: this.cur_dir
        });
        // console.log(JSON.stringify(res));
        this.update_file_list(res.files);
      } catch (err) {
        console.log(`refresh ${this.cur_dir} failed: ${JSON.stringify(err)}`);
      }
    },
    
  }
};
</script>
<style scoped >
.home{
  overflow: hidden;
}
.handle{
  padding: 0.35em;
}
.handle.material-icons{
  vertical-align: sub; 
  pointer-events: initial;
}
.fs-menu{
  position: absolute;
  /* must use left/top, or it will stretch */
  left: 60%;
  top: 40%;
  border-radius: 2em;
  background-color: rgb(230, 227, 227);
}
.fs-dropdown{
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  right: 50%;
  top: 75%;
  margin: 0;
}
.sort-dropdown{
  /* font-size: 1.06em; */
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  top: 75%;
  margin: 0;
  left: 50%;
}
.fs-dropdown > div + div, .sort-dropdown > div + div{
  border-top: 1px outset;
}
.fs-dropdown > div, .sort-dropdown > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
}
.fs-menu.is-open > div {
  display: block;
}
.cur > div:first-child {
  /* border: 1px solid red; */
  flex: 1;
}
.cur {
  /* position: fixed;
  width: 100%;
  top: 2rem; */
  align-items: center;
  display: flex;
  border: 1px solid;
  text-align: left;
  background-color: rgb(130, 233, 247);
  z-index: 7;
}
.main {
  height: calc(100vh - 6.5rem);
  overflow-y: auto;
}

.return,
.new-folder {
  border: none;
  outline: none;
  background-color: transparent;
  font-weight: 900;
  /* border-radius: 1em; */
  padding: 0.7em;
  margin-left: auto;
}


button:disabled {
  /* background-color: #ccc; */
  color: grey;
}

</style>
