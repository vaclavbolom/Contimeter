# Contimeter
Continuous metering of sensor data

# Cloudflare tunnel
## From Cloudflare portal
- create tunnel (Networking/Tunnels)
- copy-paste script to local machine and run tunnel
    - service is created and started
- add routing 
    - connect url with application
    - open tunnel
    - Navigate to 'Routes' tab
    - Creation of route creates a DNS record


## From local machine

## Issues and observations
- Grafana: grafana.contimeter.eu
- HomeAssistant: ha.contimeter.eu
    - contimeter.eu/ha does not work
    - It was necessary to add one more address range to trusted proxies
        - volume for homeassistant, confiruration file `configuration.yaml`
        - 172.18.0.0/16
        - address comes from container log (forbidden)

# Run python script as a service

1. Create script and make it executable.
2. Create Systemd Service File.
    ```
    sudo nano /etc/systemd/system/my_script.service
    ```
    ```

    [Unit]
    Description=My Python Service
    After=network.target

    [Service]
    Type=simple
    ExecStart=/usr/bin/python3 /home/username/my_script.py # Python interpreter
    WorkingDirectory=/home/username # directory where script runs
    Restart=always # script automaticaly restarts
    RestartSec=5
    User=username  # run as non root
    Environment="PYTHONUNBUFFERED=1"

    [Install]
    WantedBy=multi-user.target

    ```
1. Reload systemd to apply changes
    ```
    sudo systemctl daemon-reload
    ```

1. Enable service to start on boot
    ```
    sudo systemctl enable my_script.service
    ```

1. Start the service.
    ```
    sudo systemctl start my_script.service
    ```

1. Check status
    ```
    sudo systemctl status my_script.service
    ```

1. View logs
    ```
    journalctl -u my_script.service -f
    ```

1. Stop or restart
    ```
    sudo systemctl stop my_script.service

    sudo systemctl restart my_script.service
    ```

1. Disable service ( remove from autostart)
    ```
    sudo systemctl disable my_script.service
    ```

1. Delete service
    ```
    sudo rm /etc/systemd/system/my_script.service
    ```
    Then reload systemd

1. Remove logs from journal
    ```
    sudo journalctl --vacuum-time=1s
    ```

1. List services
    ```
    systemctl list-units --type=service --all

    systemctl list-units --type=service # only running

    systemctl | grep my_script # search for a specific service
    ```

# Run Grafana on https
**Not applicable for solution with Cloudflare tunnel. It allready contains https.**
NGINX + Let's encrypt

1. Install NGINX
    '''Shell
    sudo apt install nginx -yShow more lines
    '''
1. Configure NGINX reverse proxy
    Create a site file:
    ```Shell
    sudo nano /etc/nginx/sites-available/grafanaShow more lines
    ```
    Paste this:
    ```
    server {
        listen 80;
        server_name your-domain.com;

        location / {
            proxy_pass http://localhost:3000;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }
    }
    ```

1. Enable the config:
    ```Shell 
    sudo ln -s /etc/nginx/sites-available/grafana /etc/nginx/sites-enabled/sudo nginx -t
    sudo systemctl reload nginx
    ```


1. Install Let’s Encrypt HTTPS
    ```Shell
    sudo apt install certbot python3-certbot-nginx -yShow more lines
    ```
    Run:
    ```Shell
    sudo certbot --nginx -d your-domain.comShow more lines
    ```
    Certbot will automatically:
    ✔ generate the certificate
    ✔ configure HTTPS in NGINX
    ✔ enable auto‑renew

1.Now open:
    https://your-domain.com