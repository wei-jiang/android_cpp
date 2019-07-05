import Noty from 'noty';
import moment from "moment";
import md5 from "./md5";
import cfg from "./config";

class Util {
    show_confirm(text, ok_cb) {
        const n = new Noty({
            layout: 'center',
            text: `<div class="noty_text">${text}</div>`,
            animation: {
                open: 'animated fadeInDown', // Animate.css class names
                close: 'animated fadeOutUp' // Animate.css class names
            },
            buttons: [
                Noty.button('确认', 'btn btn-ok', function () {
                    ok_cb();
                    n.close();
                }),
                Noty.button('取消', 'btn btn-cancel', function () {
                    n.close();
                })
            ]
        });
        n.show();
    }
    md5(s){
        return md5(s);
    }
    show_success_top(text) {
        new Noty({ type: 'success', layout: 'top', text }).show();
    }
    get_dir_from_path(file_path){
        return file_path.substring( 0, file_path.lastIndexOf("/") + 1 );
    }
    get_name_from_path(file_path){
        return file_path.substring( file_path.lastIndexOf("/") + 1 );
    }
    get_order_id() {
        return `${cli_id}-${moment().format("YYYYMMDDHHmmssSSS")}`
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
    slice_n_from_end(str, n = 4){
        return str.slice( str.length - n );
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
    async post_local(url, data) {       
        const host = location.host || `127.0.0.1:${cfg.svr_port}`;
        console.log(`post_local host=${host}`)
        const res = await $.ajax({
            type: "POST",
            url: `http://${host}/${url}`,
            contentType: "application/json; charset=utf-8",
            data: JSON.stringify(data),
            dataType: "json"
        });
        return res;
    }
    toHHMMSS(s) {
        let sec_num = parseInt(s, 10); // don't forget the second param
        let hours   = Math.floor(sec_num / 3600);
        let minutes = Math.floor((sec_num - (hours * 3600)) / 60);
        let seconds = sec_num - (hours * 3600) - (minutes * 60);
    
        if (hours   < 10) {hours   = "0"+hours;}
        if (minutes < 10) {minutes = "0"+minutes;}
        if (seconds < 10) {seconds = "0"+seconds;}
        return hours+':'+minutes+':'+seconds;
    }
    path2url(path){
        return path.replace("/sdcard/mystore", this.store_url());
    }
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
    in_mobile(){
        let check = false;
        (function(a){if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino/i.test(a)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0,4))) check = true;})(navigator.userAgent||navigator.vendor||window.opera);
        return check;
    }
    in_mobile_or_tablet(){
        let check = false;
        (function(a){if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino|android|ipad|playbook|silk/i.test(a)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0,4))) check = true;})(navigator.userAgent||navigator.vendor||window.opera);
        return check;
    }
    in_wx(){
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