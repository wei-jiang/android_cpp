<template>
  <div class="hello">
    <!-- <h1>{{ msg }}</h1> -->
    <div class="progressbar" v-for="(v, k) in uploading">
      <div v-bind:style="{width: v.progress}">{{`${k}(${v.progress})`}}</div>
    </div>
    <input type="file" multiple @change="processFile($event)">
    <button @click.prevent="open_file()" v-if="up_count==0">上传文件</button>

    <div class="fi" v-for="f in files">
      <div v-if="f.file_type.includes('image/')">
        <img :src="file_url(f.path)"/>
      </div>
      <div v-else-if="f.file_type.includes('audio/')">
        <audio :src="file_url(f.path)" controls>
      </div>
      <div v-else-if="f.file_type.includes('video/')">
        <video :src="file_url(f.path)" controls="controls">
      </div>
      <div v-else>
        <a :href="file_url(f.path)">打开文件</a>
        <div>{{`文件类型：${f.file_type}`}}</div>
        <div>{{`文件大小：${formatFileSize(f.file_size)}`}}</div>
        <div>{{`文件名称：${f.filename}`}}</div>
        <div>{{`文件路径：${f.path}`}}</div>
      </div>
      <button @click.prevent="del_file(f)">删除文件</button>
    </div>
  </div>
</template>

<script>
import ws from "../ws";
export default {
  name: "ResMgr",
  props: {
    msg: String
  },
  created: function() {
    this.$root.$on("get_file_list", files => {
      this.files = files;
    });
    if (isCordovaApp) {
      document.addEventListener(
        "deviceready",
        () => {
          window.device_ready = true;
          cpp.start(
            12345,
            data => {
              this.cpp_svr_started = true;
              this.msg = data;
              ws.init();
            },
            err => {
              this.msg = err;
            }
          );
        },
        false
      );
    } else {
      ws.init();
    }
  },
  mounted() {},
  data() {
    return {
      msg: "",
      cpp_svr_started: false,
      files: [],
      uploading: {},
      up_count: 0
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
    del_file(f){
      const cmd = {
        cmd: 'del_file',
        path: f.path
      }
      ws.send( JSON.stringify(cmd) )
    },
    file_url(file_path){
      return file_path.replace('/sdcard/mystore', this.store_url)
    },
    formatFileSize(bytes, decimalPoint) {
      if (bytes == 0) return "0 Bytes";
      let k = 1000,
        dm = decimalPoint || 2,
        sizes = ["Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"],
        i = Math.floor(Math.log(bytes) / Math.log(k));
      return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + " " + sizes[i];
    },
    open_file() {
      $('input[type="file"]').click();
    },
    upload_file(file) {
      let loaded = 0;
      let step = 1048576; //1024*1024; size of one chunk
      let total = file.size; // total size of file
      let start = 0; // starting position
      let reader = new FileReader();
      let blob = file.slice(start, step); //a single chunk in starting of step size
      reader.readAsBinaryString(blob); // reading that chunk. when it read it, onload will be invoked

      reader.onload = e => {
        let flag = 0;
        if (loaded > 0 && loaded + step <= total) flag = 1;
        else if (loaded + step > total) flag = 2;
        let buff = reader.result;
        buff = Buffer.concat([
          Buffer.from(file.name),
          Buffer.from([0], "binary"),
          Buffer.from(buff, "binary"),
          Buffer.from([flag], "binary")
        ]);
        $.ajax({
          type: "POST",
          url: "/upload",
          // timeout: 3000,
          dataType: "text",
          contentType: "application/octet-stream",
          data: buff,
          processData: false
        })
          .done(r => {
            loaded += step; //increasing loaded which is being used as start position for next chunk
            if (loaded <= total) {
              // if file is not completely uploaded
              blob = file.slice(loaded, loaded + step); // getting next chunk
              reader.readAsBinaryString(blob); //reading it through file reader which will call onload again. So it will happen recursively until file is completely uploaded.
            } else {
              // if file is uploaded completely
              loaded = total; // just changed loaded which could be used to show status.
              if (--this.up_count == 0) {
                this.uploading = {};
              }
            }
            this.uploading[file.name].progress = `${parseFloat(
              (loaded / total) * 100
            ).toFixed(2)}%`;
          })
          .fail(err => {
            console.log("failed: ", err);
          });
      };
    },
    processFile(event) {
      if (event.target.files.length == 0) return;
      this.up_count = event.target.files.length;
      const ups = {};
      _.each(event.target.files, f => {
        ups[f.name] = { progress: "0%", size: f.size };
        this.upload_file(f);
      });
      this.uploading = ups;
      $('input[type="file"]').val("");
    },
    test_sock() {}
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped lang="less">
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
.progressbar {
  margin: 0.3em 0.5em;
  background-color: black;
  border-radius: 1em; /* (height of inner div) / 2 + padding */
  padding: 0.2em;
}
.progressbar + .progressbar {
  margin-top: 0.7em;
}
.progressbar > div {
  text-align: center;
  background-color: orange;
  height: 2em;
  line-height: 2em;
  border-radius: 0.7em;
  color: green;
  font-weight: bold;
}
.fi {
  background-color: aqua;
  margin: 0.3em 0.5em 0;
  border: 2px groove black;
}
.fi + .fi {
}
audio, video, img{
  width: 100%;
}
</style>
