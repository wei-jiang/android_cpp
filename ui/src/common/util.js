import Noty from 'noty';
import moment from "moment";
import uuidv1 from 'uuid/v1'
import md5 from "./md5";
// import gImg from "./gen_img";
import gFace from "./face";
import Cordova from "./cordova";
const free_seconds = 10 * 60 * 1000;
class Util extends Cordova {
    uuid() {
        return uuidv1();
    }
    restart_ads_tm() {
        window.is_ads_tm = false;
        setTimeout(() => window.is_ads_tm = true, free_seconds)
    }
    randomInt(low = 0, high = 4294967295) {
        return Math.floor(Math.random() * (high - low) + low)
    }
    md5(s) {
        return md5(s);
    }
    get_stream(constraints) {
        return new Promise((resolve, reject) => {
            navigator.getUserMedia(constraints, (stream) => {
                // console.log(`audio_chat， getUserMedia success:${stream}`)
                resolve(stream);
                // stream.getTracks().forEach(track => track.stop())
            }, () => {
                reject();
            })
        });
    }
    // outside need catch exception
    async get_audio_stream() {
        if( !this.hasUserMedia() ) throw '设备不支持获取媒体流';
        await this.requestPermission("android.permission.RECORD_AUDIO");
        return await this.get_stream({ video: false, audio: true })
    }
    async get_video_stream() {
        if( !this.hasUserMedia() ) throw '设备不支持获取媒体流';
        await this.requestPermission("android.permission.RECORD_AUDIO");
        await this.requestPermission("android.permission.CAMERA");
        // { video: { width: 800, height: 600 }, audio: true }
        return await this.get_stream({ video: true, audio: true })
    }
    get_ext_of_file(fn) {
        return fn.slice(fn.lastIndexOf("."));
    }
    gen_face() {
        return gFace()//gImg()
    }
    show_success_top(text) {
        new Noty({ type: 'success', layout: 'top', text }).show();
    }
    get_dir_from_path(file_path) {
        return file_path.substring(0, file_path.lastIndexOf("/") + 1);
    }
    requestPermission(permission) {
        return new Promise((resolve, reject) => {
            cpp.requestPermission(permission, () => resolve(), () => reject());
        });
    }
    hasUserMedia() {
        navigator.getUserMedia = navigator.getUserMedia ||
            navigator.webkitGetUserMedia || navigator.mozGetUserMedia ||
            navigator.msGetUserMedia;
        return !!navigator.getUserMedia;
    }
    get_name_from_path(file_path) {
        return file_path.substring(file_path.lastIndexOf("/") + 1);
    }
    get_order_id() {
        return `${cli_id}-${moment().format("YYYYMMDDHHmmssSSS")}`
    }
    show_confirm_top(text, ok_text, no_text, tm = 9) {
        // tm = seconds
        return new Promise((resolve, reject) => {
            // id can not begin with number
            const btn_id = `b${(new Date()).getTime()}`;
            const n = new Noty({
                text: `<h2 style="text-align:center;">${text}</h2>`,
                layout: 'topCenter',
                animation: {
                    open: 'animated bounceIn', // Animate.css class names
                    close: 'animated bounceOut' // Animate.css class names
                },
                buttons: [
                    Noty.button(ok_text, 'btn btn-success', () => {
                        resolve();
                        n.conclusion = true;
                        n.close();
                    }),

                    Noty.button(`${no_text}(${tm})`, 'btn btn-error', () => {
                        reject();
                        n.conclusion = true;
                        n.close();
                    }, { id: btn_id })
                ]
            });
            n.show();
            function count_down() {
                if (n.conclusion) return;
                setTimeout(() => {
                    // console.log(`count_down, tm=${tm}`)
                    if (--tm > 0) {
                        $(`#${btn_id}`).text(`${no_text}(${tm})`);
                        count_down();
                    } else {
                        reject();
                        n.close();
                    }
                }, 1000);
            }
            count_down();
        });
    }
    show_alert_top(text) {
        new Noty({
            layout: 'top',
            text,
            animation: {
                open: 'animated bounceIn', // Animate.css class names
                close: 'animated bounceOut' // Animate.css class names
            }
        }).show();
    }
    show_alert_top_tm(text, timeout = 3000) {
        new Noty({
            timeout,
            layout: 'top',
            text,
            animation: {
                open: 'animated pulse', // Animate.css class names
                close: 'animated slideOutUp' // Animate.css class names
            }
        }).show();
    }
    show_info_center_tm(text, timeout = 3000) {
        new Noty({ timeout, layout: 'center', text, type: 'information' }).show();
    }
    show_warn_top(text) {
        new Noty({ type: 'warning', layout: 'top', text }).show();
    }
    show_error_top(text) {
        new Noty({ type: 'error', layout: 'top', text }).show();
    }
    get_close_cnns_buff(pid, dir){
        let buf = Buffer.alloc(50);
        buf[0] = CPP_CMD.CLOSE_PEER_CNNS;
        buf[1] = dir;
        buf.write(pid, 2, 32);
        return buf;
    }
    now_str() {
        return moment().format("YYYY-MM-DD HH:mm:ss");
    }
    hash_code(str) {
        let hash = 0;
        for (var i = 0; i < str.length; i++) {
            hash = ~~(((hash << 5) - hash) + str.charCodeAt(i));
        }
        return hash;
    }
    in_app() {
        return window.location.href.startsWith("file://");
    }
    slice_n_from_end(str, n = 4) {
        return str.slice(str.length - n);
    }
    async post_json(url, data) {
        const res = await $.ajax({
            type: "POST",
            url,
            contentType: "application/json; charset=utf-8",
            data: JSON.stringify(data),
            dataType: "json"
        });
        return res;
    }
    timeout(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    async post_local(path, data, retry = 3) {
        if(retry <= 0) return;
        const host = location.host || `127.0.0.1:${this.http_port()}`;
        const url = `http://${host}/${path}`;
        // console.log(`post_local url=${url}`)
        try {
            const res = await $.ajax({
                type: "POST",
                url,
                contentType: "application/json; charset=utf-8",
                data: JSON.stringify(data),
                dataType: "json"
            });
            return res;
        } catch (error) {
            console.log(`post_local url=${url} failed, retry=${retry}`);
            await this.timeout(500);
            return await this.post_local(path, data, retry - 1)
        }       
    }
    toHHMMSS(s) {
        let sec_num = parseInt(s, 10); // don't forget the second param
        let hours = Math.floor(sec_num / 3600);
        let minutes = Math.floor((sec_num - (hours * 3600)) / 60);
        let seconds = sec_num - (hours * 3600) - (minutes * 60);

        if (hours < 10) { hours = "0" + hours; }
        if (minutes < 10) { minutes = "0" + minutes; }
        if (seconds < 10) { seconds = "0" + seconds; }
        return hours + ':' + minutes + ':' + seconds;
    }
    validateNum(input, min, max) {
        let num = +input;
        return num >= min && num <= max && input === num.toString();
    }
    check_addr(addr) {
        const parts = addr.split(":");
        if (parts.length != 2) return false;
        const ip = parts[0].split(".");
        const port = parts[1];
        return this.validateNum(port, 1, 65535) &&
            ip.length == 4 &&
            ip.every(segment => this.validateNum(segment, 0, 255));
    }
    path2url(path) {
        return path.replace("/sdcard/mystore", this.store_url());
    }
    ss_addrs() {
        return db.ss.find({});
    }
    http_port() {
        const svr_cfg = db.svr.findOne({});
        return svr_cfg.http_port;
    }
    socks_port() {
        const svr_cfg = db.svr.findOne({});
        return svr_cfg.socks_port;
    }
    // must be called after deviceready
    write_socks_pac(ip, port) {
        window.resolveLocalFileSystemURL(cordova.file.dataDirectory + "www/proxy.pac", fileEntry => {
            // console.log('file system open: ' + JSON.stringify(fileEntry) );
            fileEntry.createWriter(fileWriter => {
                fileWriter.onwriteend = function () {
                    console.log("write pac file successful...");
                };
                fileWriter.onerror = function (e) {
                    console.log("write pac file failed: " + e.toString());
                };
                const pac =
                    `function FindProxyForURL(url, host){
    return "SOCKS5 ${ip}:${port}; DIRECT";
}`;
                const dataObj = new Blob([pac], { type: 'text/plain' });
                fileWriter.write(dataObj);
            });
        }, () => { });
    }
    store_url() {
        let loc = window.location,
            store_url,
            h = loc.host;
        if (loc.protocol === "https:" || loc.protocol === "http:") {
            store_url = loc.protocol;
        } else {
            store_url = "http:";
            h = `localhost:${this.http_port()}`;
        }
        store_url += "//" + h + "/store";
        // console.log(store_url);
        return store_url;
    }
    formatFileSize(bytes, decimalPoint) {
        if (bytes == 0) return "0 Bytes";
        let k = 1000,
            dm = decimalPoint || 2,
            sizes = ["Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"],
            i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + " " + sizes[i];
    }
    truncate(str, len = 7){
        return _.truncate(str, {'length': len});
    }
    reset_cd(name) {
        db.cooldown.findAndUpdate({}, c => {
            c[name].current = c[name].caption;
            vm.$emit(name, c[name].current);
        });
    }
    start_cd(name) {
        db.cooldown.findAndUpdate({}, c => c[name].current = c[name].countdown);
        const cds = db.cooldown.findOne({})
        vm.$emit(name, cds[name].current);
    }
    is_valid_email(email) {
        const re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
        return re.test(String(email).toLowerCase());
    }
    is_valid_price(price) {
        const re = /^\d+(?:[.]\d{1,2})?$/;
        return re.test(price);
    }
    is_num(str) {
        const re = /^[0-9]+$/;
        return re.test(str);
    }
    // DataURL to blob
    dataURLToBlob(dataURL) {
        const BASE64_MARKER = ';base64,';
        if (dataURL.indexOf(BASE64_MARKER) == -1) {
            const parts = dataURL.split(',');
            const contentType = parts[0].split(':')[1];
            const raw = parts[1];
            return new File([raw], 'img.jpg', { type: contentType });
        }
        const parts = dataURL.split(BASE64_MARKER);
        const contentType = parts[0].split(':')[1];
        const raw = window.atob(parts[1]);
        const rawLength = raw.length;

        const uInt8Array = new Uint8Array(rawLength);

        for (let i = 0; i < rawLength; ++i) {
            uInt8Array[i] = raw.charCodeAt(i);
        }

        return new File([uInt8Array], 'img.jpg', { type: contentType });
    }
    // to toDataURL
    resize_img_file(img_file) {
        return new Promise((resolve, reject) => {
            const image = new Image();
            image.onload = (img_event) => {
                // Resize the image
                let canvas = document.createElement('canvas'),
                    max_size = 1024, // restrain it below 1m
                    width = image.width,
                    height = image.height;
                if (width > height) {
                    if (width > max_size) {
                        height *= max_size / width;
                        width = max_size;
                    }
                } else {
                    if (height > max_size) {
                        width *= max_size / height;
                        height = max_size;
                    }
                }
                canvas.width = width;
                canvas.height = height;
                canvas.getContext('2d').drawImage(image, 0, 0, width, height);
                const dataUrl = canvas.toDataURL('image/jpeg');
                // const resizedImage = this.dataURLToBlob(dataUrl);
                resolve(dataUrl);
            }
            image.src = img_file;
        });
    }
    in_mobile() {
        let check = false;
        (function (a) { if (/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino/i.test(a) || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0, 4))) check = true; })(navigator.userAgent || navigator.vendor || window.opera);
        return check;
    }
    in_mobile_or_tablet() {
        let check = false;
        (function (a) { if (/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino|android|ipad|playbook|silk/i.test(a) || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0, 4))) check = true; })(navigator.userAgent || navigator.vendor || window.opera);
        return check;
    }
    in_wx() {
        const ua = window.navigator.userAgent.toLowerCase();
        return this.in_mobile() && ua.match(/MicroMessenger/i) == 'micromessenger';
    }
}
if (!('toJSON' in Error.prototype)) {
    Object.defineProperty(Error.prototype, 'toJSON', {
        value: function () {
            let alt = {};

            Object.getOwnPropertyNames(this).forEach(function (key) {
                alt[key] = this[key];
            }, this);

            return alt;
        },
        configurable: true,
        writable: true
    });
}

export default new Util;