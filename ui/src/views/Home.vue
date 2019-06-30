<template>
  <div class="home" @click.prevent="close_menu">
    <div class="cur">
      <div>{{cur_dir}}</div>
      <button class="return" @click="go_back" :disabled="dir.length <= 0">
        <i class="material-icons">keyboard_backspace</i>
      </button>
      <button class="new-folder" @click="create_folder">
        <i class="material-icons">create_new_folder</i>
      </button>
    </div>
    <div class="pad-fix"></div>
    <!-- {{ $t('message') }} -->
    <div class="fi" v-for="f in files" @click="open_file_or_folder(f)">
      <div v-if="f.is_dir">
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
          <!-- <div v-if="!f.is_dir">{{f.type}}</div> -->
          <div class="file-size" v-if="!f.is_dir">{{formatFileSize(f.size)}}</div>
          <!-- <div>{{f.path}}</div> -->
        </div>
        <div v-if="!f.is_dir && f.shown">
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
  </div>
</template>

<script>
import cfg from "../common/config";
import util from "../common/util";
import ws from "../ws";
export default {
  name: "home",
  created: function() {
    this.$root.$on("get_file_list", this.on_files_chg);
    this.$root.$on("del_file", this.on_del_file);
    this.$root.$on("rename_file", this.on_rename_file);
    this.$root.$on("create_dir", this.on_create_dir);
    this.$root.$on("refresh_file_list", this.on_refresh);
  },
  destroyed() {
    this.$root.$off("get_file_list", this.on_files_chg);
    this.$root.$off("del_file", this.on_del_file);
    this.$root.$off("rename_file", this.on_rename_file);
    this.$root.$off("create_dir", this.on_create_dir);
    this.$root.$off("refresh_file_list", this.on_refresh);
  },
  mounted() {},
  data() {
    return {

      dir: [],
      files: []
    };
  },
  computed: {
    cur_dir() {
      const p = this.dir.join("/");
      if (p) return `/sdcard/mystore/${p}/`;
      return `/sdcard/mystore/`;
    },
    store_url() {
      let loc = window.location,
        store_url,
        h = loc.host;
      if (loc.protocol === "https:" || loc.protocol === "http:") {
        store_url = loc.protocol;
      } else {
        store_url = "http:";
        h = `localhost:${cfg.svr_port}`;
      }
      store_url += "//" + h + "/store";
      console.log(store_url);
      return store_url;
    }
  },
  methods: {
    go_back() {
      this.dir.pop();
      this.on_refresh();
    },
    open_file_or_folder(f) {
      if (f.is_dir) {
        this.dir.push(f.name);
        this.on_refresh();
      } else {
        f.shown = !f.shown;
        const i = this.files.indexOf(f)
        // must splice to reactive
        this.files.splice(i, 1, f)
      }
    },
    async on_refresh() {
      // console.log(`refresh ${this.cur_dir}`);
      try {
        const res = await util.post_local("get_files", {
          path: this.cur_dir
        });
        // console.log(JSON.stringify(res));
        this.files = res.files;
      } catch (err) {
        console.log(`refresh ${this.cur_dir} failed: ${JSON.stringify(err)}`);
      }
    },
    create_folder() {
      let name = prompt("新文件夹名称:", "新建文件夹");
      name = name.replace(/[\n\r]/gm, "");
      if (name) {
        const cmd = {
          cmd: "create_dir",
          path: `${this.cur_dir}${name}`
        };
        ws.send(JSON.stringify(cmd));
      }
    },
    move_to(f) {
      alert(i18n.t("hello"));
    },
    close_menu() {
      $(".op-menu").removeClass("is-open");
    },
    toggle_menu(e) {
      e.currentTarget.classList.toggle("is-open");
    },
    on_files_chg(data) {
      this.files = data.files;
    },
    on_create_dir(data) {
      if (data.ret == 0) {
        const n = util.get_name_from_path(data.path);
        util.show_info_center_tm(`创建文件夹【${n}】成功`);
      } else {
        util.show_error_top(`创建文件夹失败：${data.msg}`);
      }
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
        util.show_info_center_tm(`重命名【${n}】成功`);
      } else {
        util.show_error_top(`重命名文件失败：${data.msg}`);
      }
    },
    rename_file(f) {
      let new_name = prompt("新文件名:", f.name);
      new_name = new_name.replace(/[\n\r]/gm, "");
      if (new_name && new_name != f.name) {
        new_name = f.path.replace(f.name, new_name);
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
      const cmd = {
        cmd: "del_file",
        path: f.path
      };
      ws.send(JSON.stringify(cmd));
    },
    file_url(file_path) {
      return file_path.replace("/sdcard/mystore", this.store_url);
    },
    formatFileSize(bytes, decimalPoint) {
      if (bytes == 0) return "0 Bytes";
      let k = 1000,
        dm = decimalPoint || 2,
        sizes = ["Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"],
        i = Math.floor(Math.log(bytes) / Math.log(k));
      return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + " " + sizes[i];
    }
  }
};
</script>
<style scoped >
.cur > div:first-child {
  /* border: 1px solid red; */
  flex: 1;
}
.cur {
  position: fixed;
  width: 100%;
  top: 2rem;
  align-items: center;
  display: flex;
  border: 1px solid;
  text-align: left;
  background-color: rgb(130, 233, 247);
  z-index: 7;
}
h3 {
  margin: 40px 0 0;
}

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
  right: 0.9em;
  top: 1.3em;
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
.pad-fix {
  margin-top: 3rem;
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
