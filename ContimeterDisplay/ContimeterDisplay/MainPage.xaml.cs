using MQTTnet;



namespace ContimeterDisplay
{
    public partial class MainPage : ContentPage
    {
        int count = 0;
        int countOff = 0;
        private IMqttClient _mqttClient;
        private MqttClientOptions _mqttOptions;

        private const string Topic = "contimeter/alej1/display";
        private const string TopicCalibration = "contimeter/alej1/calibration";

        public MainPage()
        {
            InitializeComponent();
            InitMqtt();
        }

        private async void OnOnBtnClicked(object sender, EventArgs e)
        {
            count++;
            OnBtn.Text = $"On - {count}";
            StatusLabel.Text = "On";
            StatusLabel.TextColor = Colors.Green;

            await SendMessage("1");
        }

        private async void OnOffBtnClicked(object sender, EventArgs e)
        {
            countOff++;
            OffBtn.Text = $"Off - {countOff}";
            StatusLabel.Text = "Off";
            StatusLabel.TextColor = Colors.Red;

            await SendMessage("0");
        }

        private async void OnCalibrateBtnClicked(object sender, EventArgs e)
        {
            if (float.TryParse(ReferenceTemperatureEntry.Text, out float referenceTemp))
            {
                StatusLabel.Text = $"Calibrating at {referenceTemp}°C";
                StatusLabel.TextColor = Colors.Blue;

                // You can publish the reference temperature to MQTT or handle it as needed
                await SendMessage($"{{\"temperature\":{referenceTemp}}}", TopicCalibration);
            }
            else
            {
                StatusLabel.Text = "Invalid temperature value";
                StatusLabel.TextColor = Colors.Orange;
            }
        }

        private async Task SendMessage(string payload, string topic=Topic)
        {
            try
            {
                if (!_mqttClient.IsConnected)
                {
                    await _mqttClient.ConnectAsync(_mqttOptions, CancellationToken.None);
                }

                var mqttMessage = new MqttApplicationMessageBuilder()
                    .WithTopic(topic)
                    .WithPayload(payload)
                    .Build();

                await _mqttClient.PublishAsync(mqttMessage, CancellationToken.None);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        private void InitMqtt()
        {
            var factory = new MqttClientFactory();
            _mqttClient = factory.CreateMqttClient();

            _mqttOptions = new MqttClientOptionsBuilder()
                .WithTcpServer("192.168.0.115", 1883)
                .Build();
        }
    }

}
