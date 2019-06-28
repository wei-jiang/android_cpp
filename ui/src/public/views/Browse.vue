<template>
  <div class="browse">
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
    </div>
    <!-- {{ $t('hello') }} -->
  </div>
</template>

<script>

export default {
  name: 'browse',
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
        h = "localhost:12345";
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
<style scoped>
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
