import re, os

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_pc_monitor.c", encoding="utf-8") as f:
    c = f.read()

old = """static void handle_pc_data(const char *data, int len)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root) return;

    float cpu=0,gpu=0,gpu_temp=0,cpu_temp=0,mu=0,mt=0,du=0,dt=0,nu=0,nd=0;
    char hn[32]="PC",cn[48]="---",gn[48]="---",oi[48]="---";
    cJSON *i;
    #define GETNUM(f,k) do{ i=cJSON_GetObjectItem(root,k); if(cJSON_IsNumber(i)) f=(float)i->valuedouble; }while(0)
    #define GETSTR(s,k) do{ i=cJSON_GetObjectItem(root,k); if(cJSON_IsString(i)) strncpy(s,i->valuestring,sizeof(s)-1); }while(0)
    GETNUM(cpu,"cpu"); GETNUM(gpu,"gpu"); GETNUM(gpu_temp,"gpu_temp"); GETNUM(cpu_temp,"cpu_temp");
    GETNUM(mu,"mem_used"); GETNUM(mt,"mem_total"); GETNUM(du,"disk_used"); GETNUM(dt,"disk_total");
    GETNUM(nu,"net_up"); GETNUM(nd,"net_down");
    GETSTR(hn,"hostname"); GETSTR(cn,"cpu_name"); GETSTR(gn,"gpu_name"); GETSTR(oi,"os_info");
    ESP_LOGI(TAG, "PC data: cpu=%.0f%% cpu_temp=%.0fC mem=%.1f/%.1fGB host=%s", cpu, cpu_temp, mu, mt, hn);
    app_state_set_pc_data(cpu,gpu,gpu_temp,cpu_temp,mu,mt,du,dt,nu,nd,hn,cn,gn,oi,true);
    cJSON_Delete(root);
}"""

new = """static void handle_pc_data(const char *data, int len)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root) return;

    /* Check for command message */
    cJSON *cmd_item = cJSON_GetObjectItem(root, "cmd");
    if (cJSON_IsString(cmd_item) && cmd_item->valuestring) {
        if (strcmp(cmd_item->valuestring, "theme") == 0) {
            cJSON *val = cJSON_GetObjectItem(root, "value");
            if (cJSON_IsNumber(val)) {
                int t = (int)val->valuedouble;
                ESP_LOGI(TAG, "Theme switch: %d", t);
                app_state_set_theme(t);
            }
        }
        cJSON_Delete(root);
        return;
    }

    float cpu=0,gpu=0,gpu_temp=0,cpu_temp=0,mu=0,mt=0,du=0,dt=0,nu=0,nd=0;
    char hn[32]="PC",cn[48]="---",gn[48]="---",oi[48]="---";
    cJSON *i;
    #define GETNUM(f,k) do{ i=cJSON_GetObjectItem(root,k); if(cJSON_IsNumber(i)) f=(float)i->valuedouble; }while(0)
    #define GETSTR(s,k) do{ i=cJSON_GetObjectItem(root,k); if(cJSON_IsString(i)) strncpy(s,i->valuestring,sizeof(s)-1); }while(0)
    GETNUM(cpu,"cpu"); GETNUM(gpu,"gpu"); GETNUM(gpu_temp,"gpu_temp"); GETNUM(cpu_temp,"cpu_temp");
    GETNUM(mu,"mem_used"); GETNUM(mt,"mem_total"); GETNUM(du,"disk_used"); GETNUM(dt,"disk_total");
    GETNUM(nu,"net_up"); GETNUM(nd,"net_down");
    GETSTR(hn,"hostname"); GETSTR(cn,"cpu_name"); GETSTR(gn,"gpu_name"); GETSTR(oi,"os_info");
    ESP_LOGI(TAG, "PC data: cpu=%.0f%% cpu_temp=%.0fC mem=%.1f/%.1fGB host=%s", cpu, cpu_temp, mu, mt, hn);
    app_state_set_pc_data(cpu,gpu,gpu_temp,cpu_temp,mu,mt,du,dt,nu,nd,hn,cn,gn,oi,true);
    cJSON_Delete(root);
}"""

if old in c:
    c = c.replace(old, new)
    with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_pc_monitor.c", "w", encoding="utf-8") as f:
        f.write(c)
    print("app_pc_monitor.c OK", os.path.getsize("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_pc_monitor.c"), "bytes")
else:
    print("ERROR: handle_pc_data not found!")
    # Debug: show first 200 chars
    print(repr(c[:200]))