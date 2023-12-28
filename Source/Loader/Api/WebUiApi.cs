/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Text;
using System.Text.Json;
using System.Security.Cryptography;

namespace Loader
{
    public static class WebUiApi
    {
        private static readonly HttpClient Client = new HttpClient();

        public class BaseResponse
        {
        }
        
        private class CreateServerRequest : BaseResponse
        {
            public string serverName { get; set; }
            public string serverPassword { get; set; }
            public string serverGameType { get; set; }
            public string machineId { get; set; }
        }

        public class CreateServerResponse : BaseResponse
        {
            public string id { get; set; }
            public string webUsername { get; set; }
            public string webPassword { get; set; }
            public string webUrl { get; set; }
        }

        static WebUiApi()
        {
            Client.DefaultRequestHeaders.Accept.Clear();
            Client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
        }

        // This is a super shitty way of doing most of this, it should be async, but got a bunch of wierd deadlocks when
        // I tried it before, and fixing it would require me learning more C# than I want to ...

        private static ResultType DoRequest<ResultType>(HttpMethod Method, string Uri, HttpContent Content = null) 
            where ResultType : BaseResponse
        {
            ResultType Result = null;

            try
            {
                HttpRequestMessage Request = new HttpRequestMessage(Method, Uri);
                Request.Content = Content;

                HttpResponseMessage Response = Client.Send(Request);
                if (!Response.IsSuccessStatusCode)
                {
                    Debug.WriteLine("Got error status when trying to query master server: {0}", Response.StatusCode);
                    return null;
                }

                Task<ResultType> ResponseTask = Response.Content.ReadFromJsonAsync<ResultType>();
                ResponseTask.ConfigureAwait(false);
                ResponseTask.Wait();
                if (!ResponseTask.IsCompletedSuccessfully)
                {
                    Debug.WriteLine("Got error status when trying to query master server.");
                    return null;
                }

                Result = ResponseTask.Result;
            }
            catch (Exception Ex)
            {
                Debug.WriteLine("Recieved exception when trying to get servers: {0}", Ex.Message);
            }

            return Result;
        }

        public static CreateServerResponse CreateServer(string ServerAddress, string ServerName, string ServerPassword, string ServerGameType)
        {
            // This is kinda jank, we should probably just generate a uuid and save it off the first time the user runs the application.
            string machineIdEntropy = Environment.MachineName + Environment.UserName + Environment.ProcessorCount + Environment.OSVersion;

            CreateServerRequest Request = new CreateServerRequest();
            Request.serverName = ServerName;
            Request.serverPassword = ServerPassword;
            Request.machineId = Convert.ToBase64String(SHA256.HashData(Encoding.UTF8.GetBytes(machineIdEntropy)));
            Request.serverGameType = ServerGameType;

            CreateServerResponse Result = DoRequest<CreateServerResponse>(HttpMethod.Post, ServerAddress + "/sharding", JsonContent.Create<CreateServerRequest>(Request));
            if (Result != null)
            {
                return Result;
            }
            return null;
        }
    }
}
