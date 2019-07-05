<template>
  <div class="all" @click.prevent="close_menu">
    
    <!-- {{ $t('message') }} -->
    <div class="fi" v-for="f in files" @click="open_file_or_folder(f)">
      <div v-if="f.type=='dir'">
        <i class="small material-icons">folder</i>
      </div>
      <div v-else-if="f.type.includes('image/')">
        <i class="small material-icons">image</i>
      </div>
      <div v-else-if="f.type.includes('audio/')">
        <i class="small material-icons">audiotrack</i>
      </div>
      <div v-else-if="f.type.includes('video/')">
        <i class="small material-icons">ondemand_video</i>
      </div>
      <div v-else>
        <i class="small material-icons">insert_drive_file</i>
      </div>
      <div class="file-desc">
        <div>{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
          <div class="file-size" v-if="f.type != 'dir'">{{formatFileSize(f.size)}}</div>
          <!-- <div>{{f.path}}</div> -->
        </div>
        <div v-if="f.type!='dir' && f.shown">
          <img v-if="f.type.includes('image/')" :src="file_url(f.path)" />
          <audio v-else-if="f.type.includes('audio/')" :src="file_url(f.path)" controls  @click.stop="1"/>
          <video
            v-else-if="f.type.includes('video/')"
            :src="file_url(f.path)"
            controls="controls" @click.stop="1"
          />
          <a v-else :href="file_url(f.path)" target="_blank" @click.stop="1">打开文件</a>
        </div>
      </div>
      <div class="op-menu" @click.stop="toggle_menu">
        <div class="op-btn">&#8942;</div>
        <div class="op-dropdown">
          <div @click="del_file(f)">删&nbsp;除</div>
          <div @click="rename_file(f)">重命名</div>
          <div @click="move_to(f)">移动到</div>
        </div>
      </div>
    </div>
    <div class="pad-bottom"></div>
  </div>
</template>

<script>
import cfg from "@/common/config";
import util from "@/common/util";
import ws from "@/ws";
export default {
  name: "all",
  created: function() {
    this.$root.$on("del_file", this.on_del_file);
    this.$root.$on("rename_file", this.on_rename_file);
    this.$root.$on("update_file_list", this.update_file_list);
  },
  destroyed() {
    this.$root.$off("del_file", this.on_del_file);
    this.$root.$off("rename_file", this.on_rename_file);
    this.$root.$off("update_file_list", this.update_file_list);
  },
  mounted() {

    this.files = g.files;
  },
  data() {
    return {
      files: []
    };
  },
  computed: {
    store_url() {

    }
  },
  methods: {
    open_file_or_folder(f) {
      if (f.type == 'dir') {
        this.$root.$emit("enter_dir", f.name);
      } else {
        f.shown = !f.shown;
        const i = this.files.indexOf(f)
        // must splice to make it reactive
        this.files.splice(i, 1, f)
      }
    },
    update_file_list(files) {
      this.files = files;
    },
    
    move_to(f) {
      // alert(i18n.t("hello"));
      this.$root.$emit('move_to', f);
    },
    close_menu() {
      $(".op-menu").removeClass("is-open");
    },
    toggle_menu(e) {
      e.currentTarget.classList.toggle("is-open");
    },
    on_del_file(data) {
      if (data.ret == 0) {
        const n = util.get_name_from_path(data.path);
        util.show_info_center_tm(`删除【${n}】成功`);
      } else {
        util.show_error_top(`删除文件失败：${data.msg}`);
      }
    },
    on_rename_file(data) {
      if (data.ret == 0) {
        const n = util.get_name_from_path(data.path);
        util.show_info_center_tm(`移动/重命名【${n}】成功`);
      } else {
        util.show_error_top(`移动/重命名文件失败：${data.msg}`);
      }
    },
    rename_file(f) {
      let new_name = prompt("新文件名:", f.name);
      if(new_name) new_name = new_name.replace(/[\n\r]/gm, "");
      let i = _.findIndex( g.files, ff=> ff.type == f.type && ff.name == new_name );
      if(i >= 0) return util.show_alert_top_tm('同名文件已存在')
      if (new_name && new_name != f.name) {
        new_name = util.get_dir_from_path(f.path) + new_name;
        // alert(`new_name = ${new_name}`)
        const cmd = {
          cmd: "rename_file",
          path: f.path,
          new_name
        };
        ws.send(JSON.stringify(cmd));
      }
    },
    del_file(f) {
      util.show_confirm(`确认删除【${f.name}】吗？`, ()=>{
        const cmd = {
          cmd: "del_file",
          path: f.path
        };
        ws.send(JSON.stringify(cmd));
      })
    
    },
    file_url(file_path) {
      return file_path.replace("/sdcard/mystore", util.store_url());
    },
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    }
  }
};
</script>
<style scoped >

.file-desc {
  flex: 1;
  margin: 0 0.4em;
}
.op-menu {
  position: relative;
}
.op-btn {
  font-size: 1.5em;
  margin: 0.1em 0.5em;
}
.op-dropdown {
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  right: 2.3em;
  top: 1.2em;
  margin: 0;
  z-index: 79;
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
.op-dropdown > div + div {
  border-top: 1px outset;
}
.op-dropdown > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
}
.op-menu.is-open .op-dropdown {
  display: block;
}
.file-size,
.op-menu {
  margin-left: auto;
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
/* no effect */
/* audio, video{
  pointer-events: none;
} */
audio,
video,
img {
  width: 100%;
}
</style>
