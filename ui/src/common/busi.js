
import moment from "moment";
import _ from 'lodash'
import util from "./util";
import WSS from "@/wss";

const sss = {}
class Busi {
    constructor() {
        _.bindAll(this, ['init', 'reg_evt']);
    }
    init() {
        this.reg_evt();
        this.go_pub();
    }
    go_pub() {
        try {
            const addrs = util.ss_addrs();
            // console.log(`go_pub()------------${JSON.stringify(addrs)}`)
            addrs.forEach(s => {
                if(s.enabled){
                    sss[s.addr] = new WSS(s.addr)
                }               
            })
        } catch (err) {
            console.log(`go_pub() exception`)
        }
    }
    reg_evt() {
        vm.$on("add_ss", data => {
            const addr = data.addr;
            if( !sss.hasOwnProperty(addr) ){                
                sss[addr] = new WSS(addr)
            }            
        });
        vm.$on("del_ss", data => {
            const addr = data.addr;
            if( sss.hasOwnProperty(addr) ){
                sss[addr].destroy();
                delete sss[addr];
                // noty udp off these addr
            }
        });
        // no need replace
        // vm.$on("replace_ss", data => {

        //     // vm.$emit("refresh_noty", data);
        // });
    }
}


export default new Busi;