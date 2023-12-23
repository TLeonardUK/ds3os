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

namespace Loader
{
    public static class MasterServerApi
    {
        private static readonly HttpClient Client = new HttpClient();

        private class BaseResponse
        {
            public string Status { get; set; }
            public string Message { get; set; }
        }

        private class ListServersResponse : BaseResponse
        {
            public List<ServerConfig> Servers { get; set; }
        }

        private class GetPublicKeyRequest : BaseResponse
        {
            public string Password { get; set; }
        }

        private class GetPublicKeyResponse : BaseResponse
        {
            public string PublicKey { get; set; }
        }

        static MasterServerApi()
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

                ResultType TypedResponse = ResponseTask.Result;
                if (TypedResponse.Status != "success")
                {
                    Debug.WriteLine("Got error when trying to query master server: {0}", TypedResponse.Status);
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

        public static List<ServerConfig> ListServers()
        {
            ListServersResponse Result = DoRequest<ListServersResponse>(HttpMethod.Get, ProgramSettings.Default.master_server_url + "/api/v1/servers");
            if (Result != null && Result.Servers != null)
            {
                foreach (ServerConfig config in Result.Servers)
                {
                    if (string.IsNullOrEmpty(config.Id))
                    {
                        config.Id = config.IpAddress;
                    }
                }
                return Result.Servers;
            }
            else
            {
                return null;
            }
        }

        public static string GetPublicKey(string ServerId, string Password)
        {
            GetPublicKeyRequest Request = new GetPublicKeyRequest();
            Request.Password = Password;

            GetPublicKeyResponse Result = DoRequest<GetPublicKeyResponse>(HttpMethod.Post, ProgramSettings.Default.master_server_url + "/api/v1/servers/" + ServerId + "/public_key", JsonContent.Create<GetPublicKeyRequest>(Request));
            if (Result != null)
            {
                return Result.PublicKey;
            }
            return "";
        }
    }
}
