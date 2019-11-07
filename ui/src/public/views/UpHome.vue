<template>
  <div class="up-home">
    <div v-if="uploading" class="progressbar">
      <div v-bind:style="{width: uploading.progress}"></div>
      <div class="cap">{{`${progress_cap(uploading)}(${uploading.progress})`}}</div>
    </div>
    <input type="file" @change="processFile($event)">
    <div v-if="!uploading" class="input-panel">
      <input type="password" v-model="pass" placeholder="数据库密码">
      <button class="upload_btn" @click.prevent="check_and_upload()">{{$t('upload-home')}}</button>
      <p>
        请上传网站的zip压缩包（只支持zip格式），zip解压后应直接有index.html，及其它的网站相关资源文件。<br>
        不要解压后里面还有个文件夹，然后那里面才是网站内容。
      </p>
      <a :href="api_url">后台接口说明</a>
      <a :href="home_url">打开用户网站</a>
      
    </div>
    
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";
export default {
  name: "UpHome",
  props: {
    msg: String
  },
  created: async function() {},
  destroyed() {},
  mounted() {

  },
  data() {
    return {
      pass: '',
      uploading: null,
    };
  },
  computed: {
    home_url() {
      return `http://${location.hostname}:${parseInt(location.port)+1}/`;
    },
    pass_url(){
      return `http://${location.hostname}:${parseInt(location.port)+1}/check_pass`;
    },
    api_url(){
      return `http://${location.hostname}:${parseInt(location.port)}/api.txt`;
    }
  },
  methods: {

    progress_cap(f){
      return `${_.truncate(f.name, {'length': 7})}${util.formatFileSize(f.size)}`
    },
    async check_and_upload() {
      if(!this.pass) return util.show_alert_top_tm('请输入数据库访问密码')
      try{
        const res = await util.post_json(this.pass_url, {pass: this.pass});
        if(res.ret == 0){
          $('input[type="file"]').click();
        } else {
          util.show_error_top(`无效的密码: ${res.msg}`)
        }
        this.pass = ''
      }catch(err){
        util.show_error_top(`上传网站失败: ${err}`)
      }   
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
          url: "/upload_home",
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
              this.uploading = null;
            }
            if(this.uploading){
              this.uploading.progress = `${parseFloat(
                (loaded / total) * 100
              ).toFixed(2)}%`;
            }
          })
          .fail(err => {
            console.log("failed: ", err);
          });
      };
    },
    processFile(event) {
      const file_type = /zip.*/;
      if (event.target.files.length == 0) return;
      let f = event.target.files[0];
      console.log(`f.type=${f.type}`);
      if (!f.type.match(file_type)) {
        return util.show_alert_top_tm('请选择zip文件');
      }
      this.uploading = { progress: "0%", size: f.size, name: f.name };
      this.upload_file(f);
      $('input[type="file"]').val("");
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.up-home{
  /* overflow: hidden; */
}
input[type="file"] {
  display: none;
}
input[type="password"] {
  font-size: 1.4em;
  width: 80%;
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
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translateX(-50%) translateY(-50%);
  background-color: initial;
  width: 100%;
  color: rgb(250, 163, 65);
  font-weight: bold;
}
.progressbar + .progressbar {
  margin-top: 0.7em;
}
.progressbar > div {
  text-align: center;
  background-color: rgb(87, 167, 156);
  height: 1.5em;
  line-height: 1.5em;
  border-radius: 0.7em;
}
.upload_btn {
  border-radius: 0.7em;
  display: block;
  background-color: rgb(135, 155, 155);
  color: rgb(71, 70, 70);
  font-weight: 900;
  font-size: 1.7rem;
  width: 90%;
  margin: 0.9em auto;
  text-align: center;
}
p{
  margin: 1em 0.5em;
  text-align: left;
}
a{
  display: block;
  margin: 1em;
  font-size: 1.3em;
}
.input-panel{
  margin: 0.5em;
}
</style>
