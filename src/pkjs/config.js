module.exports = [
  {
  "type": "section",
  "items": [
    {
      "type": "heading",
      "defaultValue": "Pebble Transport"
    },
    {
      "type": "text",
      "defaultValue": "Enter the NAPTAN for the stop you want. You can find this from the TSY website."
    },
    {
      "type": "input",
      "id": "naptan",
      "messageKey": "NAPTAN",
      "label": "NAPTAN"
    },
    {
      "type": "input",
      "id": "stop_name",
      "messageKey": "STOPNAME",
      "label": "Name of the stop"
    },
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
  ]
}
];