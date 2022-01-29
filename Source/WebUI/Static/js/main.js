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
var gRefreshSettingsInterval;

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

        console.log("Json: ", JSON.stringify(data));

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

    button.addEventListener('click', function() 
    {
        button.disabled = true;
        authenticate(usernameBox.value, passwordBox.value);
    });
}

// Starts async loading data to populate the page data.
function startDataRefresh()
{
    gRefreshStatisticsInterval = setInterval(refreshStatisticsTab, 5000);
    gRefreshPlayersInterval = setInterval(refreshPlayersTab, 5000);
    gRefreshSettingsInterval = setInterval(refreshSettingsTab, 5000);

    refreshStatisticsTab();
    refreshPlayersTab();
    refreshSettingsTab();
}

// Stops async loading data for the page data.
function stopDataRefresh()
{
    clearInterval(gRefreshStatisticsInterval);
    clearInterval(gRefreshPlayersInterval);
    clearInterval(gRefreshSettingsInterval);
}

// Disconnects the given player-id.
function disconnectUser(playerId)
{
    // TODO
}

// Bans the given player-id.
function bansUser(playerId)
{
    // TODO
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
                    <td class="mdl-data-table__cell--non-numeric">${player["steamId"]}</td>
                    <td class="mdl-data-table__cell--non-numeric">${player["characterName"]}</td>
                    <td>${player["soulLevel"]}</td>
                    <td>${player["souls"]}</td>
                    <td>${player["soulMemory"]}</td>
                    <td>${player["deathCount"]}</td>
                    <td>${player["multiplayCount"]}</td>
                    <td>${player["covenant"]}</td>
                    <td>${player["status"]}</td>
                    <td>${player["location"]}</td>
                    <td>${player["connectionTime"]}</td>
                    <td>
                        <button class="mdl-button mdl-js-button mdl-button--raised mdl-button--colored" onclick="disconnectUser(${player["playerId"]})">
                            Disconnect
                        </button>
                        <!--
                        <button class="mdl-button mdl-js-button mdl-button--raised mdl-button--colored" onclick="banUser(${player["playerId"]})">
                            Ban
                        </button>
                        -->
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

// Retrieves data from the server to update the settings tab.
function refreshSettingsTab()
{
}