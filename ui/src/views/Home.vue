<template>
  <div class="home">
    <div class="fi" v-for="f in files">
      <div>{{`文件名称：${f.filename}`}}</div>
      <div v-if="f.file_type.includes('image/')">
        <img :src="file_url(f.path)"/>
      </div>
      <div v-else-if="f.file_type.includes('audio/')">
        <audio :src="file_url(f.path)" controls/>
      </div>
      <div v-else-if="f.file_type.includes('video/')">
        <video :src="file_url(f.path)" controls="controls"/>
      </div>
      <div v-else>
        <a :href="file_url(f.path)">打开文件</a>
        <div>{{`文件类型：${f.file_type}`}}</div>
        <div>{{`文件大小：${formatFileSize(f.file_size)}`}}</div>
        
        <div>{{`文件路径：${f.path}`}}</div>
      </div>
      <div v-if="in_cordova">
        <button @click.prevent="del_file(f)">删除文件</button>
        <button @click.prevent="rename_file(f)">重命名</button>
      </div>
    </div>
    <!-- {{ $t('hello') }} -->
  </div>
</template>

<script>

export default {
  name: 'home',
  created: function() {
    this.$root.$on("get_file_list", this.on_files_chg);   
  },
  destroyed() {
    this.$root.$off("get_file_list", this.on_files_chg);
  },
  mounted() {},
  data() {
    return {
      files: [],
    };
  },
  computed: {
    store_url() {
      let loc = window.location,
        store_url,
        h = loc.host;
      if (loc.protocol === "https:" || loc.protocol === "http:") {
        store_url = loc.protocol;
      } else {
        store_url = "http:";
        h = "localhost:57000";
      }
      store_url += "//" + h + "/store";
      console.log(store_url);
      return store_url;
    }
  },
  methods: {
    on_files_chg(files){
      this.files = files;
    },
    rename_file(f) {
      let new_name = prompt("新文件名:", f.filename);
      if (new_name && new_name != f.filename) {
        new_name = f.path.replace(f.filename, new_name);
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
    },
    
  }
}
</script>
<style scoped >
input[type="file"] {
  display: none;
}
h3 {
  margin: 40px 0 0;
}
ul {
  list-style-type: none;
  padding: 0;
}
li {
  display: inline-block;
  margin: 0 10px;
}

.fi {
  background-color: aqua;
  margin: 0.3em 0.5em 0;
  border: 2px groove black;
}
.fi + .fi {
}
.fi > button {
  margin: 0 0.7em;
  display: inline-block;
}
audio,
video,
img {
  width: 100%;
}

</style>
