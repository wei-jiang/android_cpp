<template>
  <div class="hello">
    <!-- <h1>{{ msg }}</h1> -->
    <div class="local_ip" v-if="in_cordova">
      <div>请在内网访问以下ip或扫码打开：</div>
      <div>wifi IP：&nbsp;<i>{{ip? my_uri:'未连接'}}</i></div>
      <canvas id="uri_qr"></canvas>
    </div>
    <div v-else>
      <div class="progressbar" v-for="(v, k) in uploading">
        <div v-bind:style="{width: v.progress}"></div>
        <div class="cap">{{`${k}(${v.progress})`}}</div>
      </div>
      <input type="file" multiple @change="processFile($event)">
      <button class="upload" @click.prevent="open_file()" v-if="up_count==0">上传文件</button>
    </div>
    <div class="fi" v-for="f in files">
      <div>{{`文件名称：${f.filename}`}}</div>
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
        
        <div>{{`文件路径：${f.path}`}}</div>
      </div>
      <div v-if="in_cordova">
        <button @click.prevent="del_file(f)">删除文件</button>
        <button @click.prevent="rename_file(f)">重命名</button>
      </div>
    </div>
  </div>
</template>

<script>
import ws from "../ws";
import QRious from "qrious";
export default {
  name: "ResMgr",
  props: {
    msg: String
  },
  created: function() {
    this.$root.$on("get_file_list", files => {
      this.files = files;
    });
    this.in_cordova = isCordovaApp;
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
          networkinterface.getWiFiIPAddress(
            info => {
              this.ip = info.ip;
              const qr = new QRious({
                size: 200,
                background: "#fff",
                foreground: "#284a9f",
                element: $("#uri_qr").get(0),
                value: this.my_uri
              });
            },
            err => {}
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
      ip: "",
      msg: "",
      cpp_svr_started: false,
      files: [],
      uploading: {},
      up_count: 0
    };
  },
  computed: {
    my_uri() {
      return `http://${this.ip}:12345`;
    },
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
            if (this.up_count > 0)
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
  position: relative;
  margin: 0.3em 0.5em;
  background-color: black;
  border-radius: 1em; /* (height of inner div) / 2 + padding */
  padding: 0.2em;
  overflow: hidden;
}
.progressbar > .cap {
  // display: none;
  // z-index:8;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translateX(-50%) translateY(-50%);
  background-color: initial;
  width: 100%;
  color: green;
  font-weight: bold;
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
.upload {
  border-radius: 0.7em;
  display: block;
  background-color: chartreuse;
  color: blueviolet;
  font-size: 2rem;
  width: 80%;
  margin: 0.7em auto;
  text-align: center;
}
.local_ip {
  font-weight: 700;
  border: 1px dotted purple;
  display: flex;
  flex-flow: column;
  background-color: lightgray;
  // color: blueviolet;
}
.local_ip > canvas {
  margin: 0.3em auto;
  width: 200px;
}
i {
  color: green;
}
</style>
