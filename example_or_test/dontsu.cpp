picopico::set_channel_volume(2, 1);
picopico::set_channel_program(2, 0);
picopico::ss_smps_auto_toneoff[2] = 32 * 98;
if(f % 6 == 0){
    if((f / 6) % 8 == 0){
        picopico::sfx_kick(0);
        picopico::toneon(2, 35, 0);
    }
    if((f / 6) % 8 == 2){
        picopico::sfx_hat(0);
        picopico::toneon(2, 35, 0);
    }
    if((f / 6) % 8 == 3){
        picopico::sfx_hat(0);
        picopico::toneon(2, 35, 0);
    }
    if((f / 6) % 8 == 4){
        picopico::sfx_kick(0);
        picopico::sfx_snare(1);
        picopico::toneon(2, 30, 0);
    }
    if((f / 6) % 8 == 6){
        picopico::sfx_hat(0);
        picopico::toneon(2, 33, 0);
    }
    if((f / 6) % 8 == 7){
        picopico::sfx_hat(0);
    }
}
