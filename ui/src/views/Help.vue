<template>
  <div class="help">
    <!-- 只支持安卓5.0以上版本 -->
    <h2>使用说明</h2>
    <ol>
      <li>
        <p>
        连接wifi后启动程序，如果是安装后第一次启动，会弹出获取读写sd卡权限的请求， 点“同意”后，主界面会列出sd卡mystore目录下的所有文件和文件夹。
        </p>
      </li>
      <li>
        <p>
        点下面的“服务地址”按钮，显示当前手机的wifi ip地址，在PC中用浏览器打开这个地址，或者用其它手机扫码打开
        </p>
      </li>
      <li>
        <p>
        进入手机服务器网站后，可以浏览文件，在线播放音频、视频，也可以下载文件到本地，也可以上传本地文件到手机的sd卡去。多个用户可以同时上传、下载。<br>
        注意：上传同名文件时，后上传的会把之前的覆盖掉
        </p>
      </li>
    </ol>
    <h2>常见问答</h2>
    <ol>
      <li>
        <h3>上传的文件保存到哪里去了？</h3>
        <p>
        所有上传的文件都保存在安卓sd卡的mystore目录。应用不会触及其它目录和文件。
        </p>
      </li>
      <li>
        <h3>上传文件的大小和数量有没限制？</h3>
        <p>
        没限制，只要手机存储容量够大，别写爆了就行。上传速度与 sd卡读写速度，和内网网速有关。
        </p>
      </li>
      <li>
        <h3>其它客户端能删除我的文件吗？</h3>
        <p>
        文件的删除、移动、重命名等操作只在手机服务端开放，客户端只能浏览、下载、播放（用浏览器）音频/视频，和上传文件。
        </p>
      </li>
      <li>
        <h3>文件的上传、下载会消耗我的流量吗？</h3>
        <p>
        文件的上传、下载都是用内网流量（因为必须要连WiFi才能用），不会用到公网流量。与其他玩家交互会用到外网流量。实际上不用插sim卡，只连wifi就可使用。
        </p>
      </li>
      <li>
        <h3>选择图片、音频、视频分类时会显示所有子目录下该类型的文件吗？</h3>
        <p>
        不会显示子目录中的内容，只显示【当前目录】中的文件。
        【当前目录】显示在“文件管理”上面的标题栏，点<i class="yellow material-icons">folder</i>进入子目录，点<i class="green material-icons">keyboard_backspace</i>返回上级目录。
        点右上角的<i class="yellow-new material-icons">create_new_folder</i>在当前目录中新建文件夹。<br>
        <span>提示：</span>如果选择文件分类后，只会显示”当前目录“下该类型的文件，不会显示子文件夹。如果要进入其它文件夹，请先选择”全部“，再点击进入子文件夹。
        </p>
      </li>
      <li>
        <h3>加入省电白名单后，不接电源过段时间服务也不可访问吗？</h3>
        <p>
        有些深度定制/深度优化的安卓系统，只要应用一进入后台或锁屏，系统就断开网络了。这些系统就必须要插上电源才能当服务器使用了。
        </p>
      </li>
      <li>
        <h3>可以改服务的监听端口吗？</h3>
        <p>
        可以，点“服务地址”页面中的端口号，一个手指的青色标记那里，在弹出的对话框中改端口号即可。
        </p>
      </li>
      <li>
        <h3>怎样使用自定义的pac？</h3>
        <p>
        编写任意内容的pac文件，比如叫:my.pac，上传至/sdcard/mystore目录，然后通过http://你的安卓ip:端口/store/my.pac访问即可。
        </p>
      </li>
    </ol>
  </div>
</template>
<script>
export default {
  name: "Help",
  created: function() {

  },
  destroyed() {

  },
  mounted() {

  },
  beforeRouteEnter(to, from, next) {
    if(navigator.language !== 'zh-CN') {
      next('/help-en');
    } else {
      next()
    }
  },
  data() {
    return {
    };
  },
  computed: {

  },
  methods: {
  
  }
};
</script>
<style scoped>
.faq{
  margin-top: 1em;
}

p, h3{
  text-align: left;
  /* margin: 1.0em 0.1em; */
}
span {
  color: red;
}
.to-apply{
  margin: 1.9em 1em;
  font-size: 1.7rem;
}

</style>