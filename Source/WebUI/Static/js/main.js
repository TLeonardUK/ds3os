/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

var gRefreshStatisticsInterval;
var gRefreshPlayersInterval;
var gRefreshBansInterval;
var gRefreshDebugInterval;

var gActivePlayersChart;

window.onload = function() 
{
    onDocumentLoaded();
};

// Sets a cookie by name.
function setCookie(cname, cvalue, exdays) 
{
    var d = new Date();
    d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));

    var expires = "expires="+ d.toUTCString();
    document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
}

// Gets a cookie by name.
function getCookie(name) 
{
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) 
    {
        return parts.pop().split(';').shift();
    }
    return "";
}

// Runs on page, checks authentication
// and begins refreshing the relevant data.
function onDocumentLoaded()
{
    var logoutButton = document.querySelector('#logout-button');    
    logoutButton.addEventListener('click', function() 
    {
        storeAuthToken("");
        reauthenticate();
    });
    
    var playersChart = document.querySelector('#players-chart');    
    gActivePlayersChart = new Chart(playersChart, {
        type: 'line',
        data: {
            labels: [
                '00:00'   
            ],
            datasets: [{
                label: 'Active Players',
                data: [0],
                backgroundColor: [ 'rgba(255, 99, 132, 0.2)' ],
                borderColor: [ 'rgba(255, 99, 132, 1)' ],
                borderWidth: 1
            }]
        },
        options: {
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });

    checkAuthState();
}

// Stores authentication token in cookie.
function storeAuthToken(token)
{
    setCookie("auth-token", token);
}

// Loads authentication token from cookie.
function getAuthToken()
{
    return getCookie("auth-token");
}

// Checks with the server if we are authenticated.
// If we are this starts refreshing data, if it doesn't it presents
// an authentication dialog.
function checkAuthState()
{
    stopDataRefresh();

    fetch("/auth", 
    {
        method: 'get',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        }
    })
    .then(function (data) 
    {
        if (data.status == 200)
        {
            console.log('Request succeeded with JSON response', data);
            startDataRefresh();
        }
        else
        {
            console.log('Request failed with status', data.status);
            reauthenticate();
        }
    })
    .catch(function (error) 
    {
        console.log('Request failed', error);
        reauthenticate();
    });
}

function authenticate(username, password)
{
    var dialog = document.querySelector("#auth-dialog");   

    fetch("/auth", 
    {
        method: 'post',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        },
        body: JSON.stringify({ 
            "username": username, 
            "password": password 
        })
    })
    .then(response => 
    {        
        return response.json();
    })
    .then(function (data) 
    {
        dialog.close();       

        storeAuthToken(data["token"]);

        console.log('Request succeeded with JSON response', data);
        startDataRefresh();   
    })
    .catch(function (error) 
    {
        console.log('Request failed', error);
                
        dialog.close(); 
        reauthenticate();           
    });
}

// Shows a dialog to the user asking them to 
// authenticate with the server.
function reauthenticate()
{
    console.log("Showing reauth dialog ...");

    storeAuthToken("");

    var dialog = document.querySelector("#auth-dialog");   
    var button = document.querySelector('#auth-login-button');
    var usernameBox = document.querySelector('#auth-username');
    var passwordBox = document.querySelector('#auth-password');
    
    button.disabled = false;
    if (!dialog.showModal) 
    {
        dialogPolyfill.registerDialog(dialog);
    }
    dialog.showModal();

    var handler = function() 
    {
        button.removeEventListener('client', handler);
        button.disabled = true;
        authenticate(usernameBox.value, passwordBox.value);
    };
    button.addEventListener('click', handler);
}

// Starts async loading data to populate the page data.
function startDataRefresh()
{
    gRefreshStatisticsInterval = setInterval(refreshStatisticsTab, 5000);
    gRefreshPlayersInterval = setInterval(refreshPlayersTab, 5000);
    gRefreshBansInterval = setInterval(refreshBansTab, 5000);
    gRefreshDebugInterval = setInterval(refreshDebugTab, 5000);

    refreshStatisticsTab();
    refreshPlayersTab();
    refreshBansTab();
    refreshSettingsTab();
    refreshDebugTab();
}

// Stops async loading data for the page data.
function stopDataRefresh()
{
    clearInterval(gRefreshStatisticsInterval);
    clearInterval(gRefreshPlayersInterval);
    clearInterval(gRefreshBansInterval);
    clearInterval(gRefreshDebugInterval);
}

// Disconnects the given player-id.
function disconnectUser(playerId)
{
    console.log("Disconnecting user " + playerId);

    fetch("/players", 
    {
        method: 'delete',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        },
        body: JSON.stringify({ 
            "playerId": playerId,
            "ban": false
        })
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

// Bans the given player-id.
function banUser(playerId)
{
    console.log("Banning user " + playerId);

    fetch("/players", 
    {
        method: 'delete',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        },
        body: JSON.stringify({ 
            "playerId": playerId,
            "ban": true
        })
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

// Bans the given steam-id
function removeBan(steamId)
{
    console.log("Unbanning user " + steamId);

    fetch("/bans", 
    {
        method: 'delete',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        },
        body: JSON.stringify({ 
            "steamId": steamId
        })
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

// Sends a message to a given user.
function sendUserMessageInternal(playerId, message)
{
    console.log("Sending to user ("+playerId+"): "+message);

    fetch("/message", 
    {
        method: 'post',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        },
        body: JSON.stringify({ 
            "playerId": playerId,
            "message": message
        })
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

function sendUserMessage(playerId)
{    
    var dialog = document.querySelector("#send-message-dialog");   
    var sendButton = document.querySelector('#send-message-button');   
    var cancelButton = document.querySelector('#cancel-send-message-button');
    var messageBox = document.querySelector('#send-message-text');
    
    if (!dialog.showModal) 
    {
        dialogPolyfill.registerDialog(dialog);
    }
    dialog.showModal();

    var sendHandler = function() 
    {
        sendButton.removeEventListener('click', sendHandler);
        sendUserMessageInternal(playerId, messageBox.value);
        dialog.close();
    };
    var cancelHandler = function() 
    {
        cancelButton.removeEventListener('click', cancelHandler);
        dialog.close();
    };

    sendButton.addEventListener('click', sendHandler);
    cancelButton.addEventListener('click', cancelHandler);
}

function sendMessageToAllUsers()
{
    var dialog = document.querySelector("#send-message-dialog");   
    var sendButton = document.querySelector('#send-message-button');   
    var cancelButton = document.querySelector('#cancel-send-message-button');
    var messageBox = document.querySelector('#send-message-text');
    
    if (!dialog.showModal) 
    {
        dialogPolyfill.registerDialog(dialog);
    }
    dialog.showModal();

    var sendHandler = function() 
    {
        sendButton.removeEventListener('click', sendHandler);
        sendUserMessageInternal(0, messageBox.value);
        dialog.close();
    };
    var cancelHandler = function() 
    {
        cancelButton.removeEventListener('click', cancelHandler);
        dialog.close();
    };

    sendButton.addEventListener('click', sendHandler);
    cancelButton.addEventListener('click', cancelHandler);
}

// Retrieves data from the server to update the statistics tab.
function refreshStatisticsTab()
{
    fetch("/statistics", 
    {
        method: 'get',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        }
    })
    .then(response => 
    {        
        return response.json();
    })
    .then(function (data) 
    {
        // Update active players chart.
        var chartLabels = [];
        var chartData = [];

        for (var i = 0; i < data.activePlayerSamples.length; i++)
        {
            chartLabels.push(data.activePlayerSamples[i].time);
            chartData.push(data.activePlayerSamples[i].players);
        }

        gActivePlayersChart.data.labels = chartLabels;
        gActivePlayersChart.data.datasets[0].data = chartData;
        gActivePlayersChart.update();

        // Update the statistics list.        
        var statisticsTable = document.querySelector("#statistic-table-body");   
        var newHtml = "";

        for (var i = 0; i < data.statistics.length; i++) 
        {
            var stat = data.statistics[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric">${stat["name"]}</td>
                    <td>${stat["value"]}</td>
                </tr>
            `;
        }

        statisticsTable.innerHTML = newHtml;

        // Update populated areas list.
        var populatedAreasTable = document.querySelector("#populated-areas-table-body");   
        newHtml = "";

        for (var i = 0; i < data.populatedAreas.length; i++) 
        {
            var stat = data.populatedAreas[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric">${stat["areaName"]}</td>
                    <td>${stat["playerCount"]}</td>
                </tr>
            `;
        }

        populatedAreasTable.innerHTML = newHtml;
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

// Retrieves data from the server to update the players tab.
function refreshPlayersTab()
{
    fetch("/players", 
    {
        method: 'get',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        }
    })
    .then(response => 
    {        
        return response.json();
    })
    .then(function (data) 
    {
        var table = document.querySelector("#players-table-body");    
        var newHtml = "";

        for (var i = 0; i < data.players.length; i++) 
        {
            var player = data.players[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric"><a href="https://steamcommunity.com/profiles/${player["steamId64"]}">${player["characterName"]}</a></td>
                    <td>${player["soulLevel"]}</td>
                    <td>${player["soulMemory"]}</td>
                    <td>${player["covenant"]}</td>
                    <td>${player["status"]}</td>
                    <td>${player["location"]}</td>
                    <td>${player["playTime"]}</td>
                    <td>${player["connectionTime"]}</td>
                    <td>${player["antiCheatScore"]}</td>
                    <td>
                        <button class="mdl-button mdl-js-button mdl-button--raised mdl-button--colored" onclick="disconnectUser(${player["playerId"]})">
                            Disconnect
                        </button>
                        <button class="mdl-button mdl-js-button mdl-button--raised mdl-button--colored" onclick="banUser(${player["playerId"]})">
                            Ban
                        </button>
                        <button class="mdl-button mdl-js-button mdl-button--raised mdl-button--colored" onclick="sendUserMessage(${player["playerId"]})">
                            Message
                        </button>
                    </td>
                </tr>
            `;
        }

        table.innerHTML = newHtml;
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

// Retrieves data from the server to update the bans tab.
function refreshBansTab()
{
    fetch("/bans", 
    {
        method: 'get',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        }
    })
    .then(response => 
    {        
        return response.json();
    })
    .then(function (data) 
    {
        var table = document.querySelector("#bans-table-body");    
        var newHtml = "";

        for (var i = 0; i < data.bans.length; i++)
        {
            var ban = data.bans[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric"><a href="https://steamcommunity.com/profiles/${ban["steamId64"]}">${ban["steamId64"]}</a></td>
                    <td><div style="white-space: pre-line">${ban["reason"]}</div></td>
                    <td>
                        <button class="mdl-button mdl-js-button mdl-button--raised mdl-button--colored" onclick="removeBan('${ban["steamId"]}')">
                            Remove
                        </button>
                    </td>
                </tr>
            `;
        }

        table.innerHTML = newHtml;
    })
    .catch(function (error) 
    {
        console.log('Request failed ' + error);                
        reauthenticate();           
    });
}

// Retrieves data from the server to update the settings tab.
function refreshSettingsTab()
{
    fetch("/settings", 
    {
        method: 'get',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        }
    })
    .then(response => 
    {        
        return response.json();
    })
    .then(function (data) 
    {
        setMaterialTextField(document.querySelector("#server-name"), data.serverName);
        setMaterialTextField(document.querySelector("#server-description"), data.serverDescription);
        setMaterialTextField(document.querySelector("#server-password"), data.password);
        setMaterialTextField(document.querySelector("#server-private-hostname"), data.privateHostname);
        setMaterialTextField(document.querySelector("#server-public-hostname"), data.publicHostname);

        setMaterialCheckState(document.querySelector("#advertise"), data.advertise);
        setMaterialCheckState(document.querySelector("#disable-coop"), data.disableCoop);
        setMaterialCheckState(document.querySelector("#disable-blood-messages"), data.disableBloodMessages);
        setMaterialCheckState(document.querySelector("#disable-blood-stains"), data.disableBloodStains);
        setMaterialCheckState(document.querySelector("#disable-ghosts"), data.disableGhosts);
        setMaterialCheckState(document.querySelector("#disable-invasions"), data.disableInvasions);
        setMaterialCheckState(document.querySelector("#disable-auto-summon-coop"), data.disableAutoSummonCoop);
        setMaterialCheckState(document.querySelector("#disable-auto-summon-invasions"), data.disableAutoSummonInvasions);
        setMaterialCheckState(document.querySelector("#disable-weapon-level-matching"), data.disableWeaponLevelMatching);
        setMaterialCheckState(document.querySelector("#disable-soul-level-matching"), data.disableSoulLevelMatching);
        setMaterialCheckState(document.querySelector("#disable-soul-memory-matching"), data.disableSoulMemoryMatching);
        setMaterialCheckState(document.querySelector("#ignore-invasion-area-filter"), data.ignoreInvasionAreaFilter);
        setMaterialCheckState(document.querySelector("#anti-cheat-enabled"), data.antiCheatEnabled);
    })
    .catch(function (error) 
    {
        console.log('Request failed', error);                
        reauthenticate();           
    });
}

// Posts all the settings to the server.
function saveSettings()
{
    fetch("/settings", 
    {
        method: 'post',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        },
        body: JSON.stringify({ 
            "serverName": document.querySelector("#server-name").value, 
            "serverDescription": document.querySelector("#server-description").value,
            "password": document.querySelector("#server-password").value,
            "privateHostname": document.querySelector("#server-private-hostname").value,
            "publicHostname": document.querySelector("#server-public-hostname").value,
            
            "advertise": document.querySelector("#advertise").checked,
            "disableCoop": document.querySelector("#disable-coop").checked,
            "disableBloodMessages": document.querySelector("#disable-blood-messages").checked,
            "disableBloodStains": document.querySelector("#disable-blood-stains").checked,
            "disableGhosts": document.querySelector("#disable-ghosts").checked,
            "disableInvasions": document.querySelector("#disable-invasions").checked,
            "disableAutoSummonCoop": document.querySelector("#disable-auto-summon-coop").checked,
            "disableAutoSummonInvasions": document.querySelector("#disable-auto-summon-invasions").checked,
            "disableWeaponLevelMatching": document.querySelector("#disable-weapon-level-matching").checked,
            "disableSoulLevelMatching": document.querySelector("#disable-soul-level-matching").checked,    
            "disableSoulMemoryMatching": document.querySelector("#disable-soul-memory-matching").checked,    
            "ignoreInvasionAreaFilter": document.querySelector("#ignore-invasion-area-filter").checked,
            "antiCheatEnabled": document.querySelector("#anti-cheat-enabled").checked,
        })
    })
    .catch(function (error) 
    {
        console.log('Request failed');                
        reauthenticate();           
    });
}

function setMaterialCheckState(element, state)
{
    if (element.checked != state)
    {
        if (state)
        {
            element.parentNode.MaterialSwitch.on();
        }
        else
        {
            element.parentNode.MaterialSwitch.off();
        }
    }
}

function setMaterialTextField(element, text)
{
    element.parentNode.MaterialTextfield.change(text);
}

// Retrieves data from the server to update the debug statistics tab.
function refreshDebugTab()
{
    fetch("/debug_statistics", 
    {
        method: 'get',
        headers: 
        {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8",
            "Auth-Token": getAuthToken() 
        }
    })
    .then(response => 
    {        
        return response.json();
    })
    .then(function (data) 
    {
        var timerTable = document.querySelector("#debug-timer-table-body");   
        var counterTable = document.querySelector("#debug-counter-table-body");   
        var logTable = document.querySelector("#debug-log-table-body");   

        // Update the timer list.      
        var newHtml = "";
        for (var i = 0; i < data.timers.length; i++) 
        {
            var stat = data.timers[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric">${stat["name"]}</td>
                    <td>${stat["current"]}</td>
                    <td>${stat["average"]}</td>
                    <td>${stat["peak"]}</td>
                </tr>
            `;
        }
        timerTable.innerHTML = newHtml;
        
        // Update the counter list.      
        newHtml = "";
        for (var i = 0; i < data.counters.length; i++) 
        {
            var stat = data.counters[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric">${stat["name"]}</td>
                    <td>${stat["average_rate"]}</td>
                    <td>${stat["total_lifetime"]}</td>
                </tr>
            `;
        }
        counterTable.innerHTML = newHtml;
        
        // Update the debug log list.    
        newHtml = "";
        for (var i = 0; i < data.logs.length; i++) 
        {
            var stat = data.logs[i];
            newHtml += `        
                <tr>
                    <td class="mdl-data-table__cell--non-numeric">${stat["level"]}</td>
                    <td style="text-align:left;">${stat["source"]}</td>
                    <td style="text-align:left;">${stat["message"]}</td>
                </tr>
            `;
        }
        logTable.innerHTML = newHtml;
    })
    .catch(function (error) 
    {
        console.log('Request failed (debug_statistics): ' + error);                
        reauthenticate();           
    });
}